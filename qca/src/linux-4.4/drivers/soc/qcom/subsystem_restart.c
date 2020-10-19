/* Copyright (c) 2011-2015, 2020 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "subsys-restart: %s(): " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/suspend.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/idr.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <soc/qcom/subsystem_restart.h>
#include <soc/qcom/subsystem_notif.h>

#include <asm/current.h>

#define BUF_SIZE 30

#define DISABLE_SSR 0x9889deed
/* If set to 0x9889deed, call to subsystem_restart_dev() returns immediately */
static uint disable_restart_work;
module_param(disable_restart_work, uint, S_IRUGO | S_IWUSR);

static int enable_debug;
module_param(enable_debug, int, S_IRUGO | S_IWUSR);

static struct subsys_device *desc_to_subsys(struct device *d);

/* The maximum shutdown timeout is the product of MAX_LOOPS and DELAY_MS. */
#define SHUTDOWN_ACK_MAX_LOOPS	100
#define SHUTDOWN_ACK_DELAY_MS	100

/**
 * enum p_subsys_state - state of a subsystem (private)
 * @SUBSYS_NORMAL: subsystem is operating normally
 * @SUBSYS_CRASHED: subsystem has crashed and hasn't been shutdown
 * @SUBSYS_RESTARTING: subsystem has been shutdown and is now restarting
 *
 * The 'private' side of the subsytem state used to determine where in the
 * restart process the subsystem is.
 */
enum p_subsys_state {
	SUBSYS_NORMAL,
	SUBSYS_CRASHED,
	SUBSYS_RESTARTING,
};


static const char * const subsys_states[] = {
	[SUBSYS_OFFLINE] = "OFFLINE",
	[SUBSYS_ONLINE] = "ONLINE",
};

static const char * const restart_levels[] = {
	[RESET_SOC] = "SYSTEM",
	[RESET_SUBSYS_COUPLED] = "RELATED",
};

/**
 * struct subsys_tracking - track state of a subsystem or restart order
 * @p_state: private state of subsystem/order
 * @state: public state of subsystem/order
 * @s_lock: protects p_state
 * @lock: protects subsystem/order callbacks and state
 *
 * Tracks the state of a subsystem or a set of subsystems (restart order).
 * Doing this avoids the need to grab each subsystem's lock and update
 * each subsystems state when restarting an order.
 */
struct subsys_tracking {
	enum p_subsys_state p_state;
	spinlock_t s_lock;
	enum subsys_state state;
	struct mutex lock;
};

/**
 * struct subsys_soc_restart_order - subsystem restart order
 * @subsystem_list: names of subsystems in this restart order
 * @count: number of subsystems in order
 * @track: state tracking and locking
 * @subsys_ptrs: pointers to subsystems in this restart order
 */
struct subsys_soc_restart_order {
	struct device_node **device_ptrs;
	int count;

	struct subsys_tracking track;
	struct subsys_device **subsys_ptrs;
	struct list_head list;
};

struct restart_log {
	struct timeval time;
	struct subsys_device *dev;
	struct list_head list;
};

/**
 * struct subsys_device - subsystem device
 * @desc: subsystem descriptor
 * @work: context for subsystem_restart_wq_func() for this device
 * @ssr_wlock: prevents suspend during subsystem_restart()
 * @wlname: name of wakeup source
 * @device_restart_work: work struct for device restart
 * @track: state tracking and locking
 * @notify: subsys notify handle
 * @dev: device
 * @owner: module that provides @desc
 * @count: reference count of subsystem_get()/subsystem_put()
 * @id: ida
 * @restart_level: restart level (0 - panic, 1 - related, 2 - independent, etc.)
 * @restart_order: order of other devices this devices restarts with
 * @crash_count: number of times the device has crashed
 * @do_ramdump_on_put: ramdump on subsystem_put() if true
 * @crashed: indicates if subsystem has crashed
 * @notif_state: current state of subsystem in terms of subsys notifications
 */
struct subsys_device {
	struct subsys_desc *desc;
	struct work_struct work;
	struct wakeup_source ssr_wlock;
	char wlname[64];
	struct work_struct device_restart_work;
	struct subsys_tracking track;

	void *notify;
	struct device dev;
	struct module *owner;
	int count;
	int id;
	int restart_level;
	int crash_count;
	struct subsys_soc_restart_order *restart_order;
	bool do_ramdump_on_put;
	struct cdev char_dev;
	dev_t dev_no;
	bool crashed;
	int notif_state;
	struct list_head list;
};

static struct subsys_device *to_subsys(struct device *d)
{
	return container_of(d, struct subsys_device, dev);
}

static struct subsys_tracking *subsys_get_track(struct subsys_device *subsys)
{
	struct subsys_soc_restart_order *order = subsys->restart_order;

	if (order)
		return &order->track;
	else
		return &subsys->track;
}

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", to_subsys(dev)->desc->name);
}

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	enum subsys_state state = to_subsys(dev)->track.state;
	return snprintf(buf, PAGE_SIZE, "%s\n", subsys_states[state]);
}

static ssize_t crash_count_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", to_subsys(dev)->crash_count);
}

static ssize_t
restart_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int level = to_subsys(dev)->restart_level;
	return snprintf(buf, PAGE_SIZE, "%s\n", restart_levels[level]);
}

static ssize_t restart_level_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct subsys_device *subsys = to_subsys(dev);
	const char *p;
	int i, orig_count = count;

	p = memchr(buf, '\n', count);
	if (p)
		count = p - buf;

	for (i = 0; i < ARRAY_SIZE(restart_levels); i++)
		if (!strncasecmp(buf, restart_levels[i], count)) {
			subsys->restart_level = i;
			return orig_count;
		}
	return -EPERM;
}

static ssize_t firmware_name_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", to_subsys(dev)->desc->fw_name);
}

static ssize_t firmware_name_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct subsys_device *subsys = to_subsys(dev);
	struct subsys_tracking *track = subsys_get_track(subsys);
	const char *p;
	int orig_count = count;

	p = memchr(buf, '\n', count);
	if (p)
		count = p - buf;

	pr_info("Changing subsys fw_name to %s\n", buf);
	mutex_lock(&track->lock);
	strlcpy(subsys->desc->fw_name, buf,
			min(count + 1, sizeof(subsys->desc->fw_name)));
	mutex_unlock(&track->lock);
	return orig_count;
}

static ssize_t system_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct subsys_device *subsys = to_subsys(dev);
	char p[6] = "set";

	if (!subsys->desc->system_debug)
		strlcpy(p, "reset", sizeof(p));

	return snprintf(buf, PAGE_SIZE, "%s\n", p);
}

static ssize_t system_debug_store(struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t count)
{
	struct subsys_device *subsys = to_subsys(dev);
	const char *p;
	int orig_count = count;

	p = memchr(buf, '\n', count);
	if (p)
		count = p - buf;

	if (!strncasecmp(buf, "set", count))
		subsys->desc->system_debug = true;
	else if (!strncasecmp(buf, "reset", count))
		subsys->desc->system_debug = false;
	else
		return -EPERM;
	return orig_count;
}

int subsys_get_restart_level(struct subsys_device *dev)
{
	return dev->restart_level;
}
EXPORT_SYMBOL(subsys_get_restart_level);

enum subsys_state subsys_get_state(struct subsys_device *subsys)
{
	enum subsys_state state = subsys->track.state;
	return state;
}
EXPORT_SYMBOL(subsys_get_state);

static void subsys_set_state(struct subsys_device *subsys,
			     enum subsys_state state)
{
	unsigned long flags;

	spin_lock_irqsave(&subsys->track.s_lock, flags);
	if (subsys->track.state != state) {
		subsys->track.state = state;
		spin_unlock_irqrestore(&subsys->track.s_lock, flags);
		sysfs_notify(&subsys->dev.kobj, NULL, "state");
		return;
	}
	spin_unlock_irqrestore(&subsys->track.s_lock, flags);
}

/**
 * subsytem_default_online() - Mark a subsystem as online by default
 * @dev: subsystem to mark as online
 *
 * Marks a subsystem as "online" without increasing the reference count
 * on the subsystem. This is typically used by subsystems that are already
 * online when the kernel boots up.
 */
void subsys_default_online(struct subsys_device *dev)
{
	subsys_set_state(dev, SUBSYS_ONLINE);
}
EXPORT_SYMBOL(subsys_default_online);

static struct device_attribute subsys_attrs[] = {
	__ATTR_RO(name),
	__ATTR_RO(state),
	__ATTR_RO(crash_count),
	__ATTR(restart_level, 0644, restart_level_show, restart_level_store),
	__ATTR(firmware_name, 0644, firmware_name_show, firmware_name_store),
	__ATTR(system_debug, 0644, system_debug_show, system_debug_store),
	__ATTR_NULL,
};

struct bus_type subsys_bus_type = {
	.name		= "msm_subsys",
	.dev_attrs	= subsys_attrs,
};
EXPORT_SYMBOL(subsys_bus_type);

static DEFINE_IDA(subsys_ida);

static int enable_ramdumps;
module_param(enable_ramdumps, int, S_IRUGO | S_IWUSR);

static int enable_mini_ramdumps;
module_param(enable_mini_ramdumps, int, S_IRUGO | S_IWUSR);

struct workqueue_struct *ssr_wq;
static struct class *char_class;

static LIST_HEAD(restart_log_list);
static LIST_HEAD(subsys_list);
static LIST_HEAD(ssr_order_list);
static DEFINE_MUTEX(soc_order_reg_lock);
static DEFINE_MUTEX(restart_log_mutex);
static DEFINE_MUTEX(subsys_list_lock);
static DEFINE_MUTEX(char_device_lock);
static DEFINE_MUTEX(ssr_order_mutex);

static struct subsys_soc_restart_order *
update_restart_order(struct subsys_device *dev)
{
	int i;
	struct subsys_soc_restart_order *order;
	struct device_node *device = dev->desc->dev->of_node;

	mutex_lock(&soc_order_reg_lock);
	list_for_each_entry(order, &ssr_order_list, list) {
		for (i = 0; i < order->count; i++) {
			if (order->device_ptrs[i] == device) {
				order->subsys_ptrs[i] = dev;
				goto found;
			}
		}
	}
	order = NULL;
found:
	mutex_unlock(&soc_order_reg_lock);

	return order;
}

static int max_restarts;
module_param(max_restarts, int, 0644);

static long max_history_time = 3600;
module_param(max_history_time, long, 0644);

static void do_epoch_check(struct subsys_device *dev)
{
	int n = 0;
	struct timeval *time_first = NULL, *curr_time;
	struct restart_log *r_log, *temp;
	static int max_restarts_check;
	static long max_history_time_check;

	mutex_lock(&restart_log_mutex);

	max_restarts_check = max_restarts;
	max_history_time_check = max_history_time;

	/* Check if epoch checking is enabled */
	if (!max_restarts_check)
		goto out;

	r_log = kmalloc(sizeof(struct restart_log), GFP_KERNEL);
	if (!r_log)
		goto out;
	r_log->dev = dev;
	do_gettimeofday(&r_log->time);
	curr_time = &r_log->time;
	INIT_LIST_HEAD(&r_log->list);

	list_add_tail(&r_log->list, &restart_log_list);

	list_for_each_entry_safe(r_log, temp, &restart_log_list, list) {

		if ((curr_time->tv_sec - r_log->time.tv_sec) >
				max_history_time_check) {

			pr_debug("Deleted node with restart_time = %ld\n",
					r_log->time.tv_sec);
			list_del(&r_log->list);
			kfree(r_log);
			continue;
		}
		if (!n) {
			time_first = &r_log->time;
			pr_debug("Time_first: %ld\n", time_first->tv_sec);
		}
		n++;
		pr_debug("Restart_time: %ld\n", r_log->time.tv_sec);
	}

	if (time_first && n >= max_restarts_check) {
		if ((curr_time->tv_sec - time_first->tv_sec) <
				max_history_time_check)
			panic("Subsystems have crashed %d times in less than "
				"%ld seconds!", max_restarts_check,
				max_history_time_check);
	}

out:
	mutex_unlock(&restart_log_mutex);
}

static int is_ramdump_enabled(struct subsys_device *dev)
{
	if (dev->desc->ramdump_disable_gpio)
		return !dev->desc->ramdump_disable;

	return enable_ramdumps;
}

static void for_each_subsys_device(struct subsys_device **list, unsigned count,
		void *data, void (*fn)(struct subsys_device *, void *))
{
	while (count--) {
		struct subsys_device *dev = *list++;
		if (!dev)
			continue;
		fn(dev, data);
	}
}

static void notify_each_subsys_device(struct subsys_device **list,
		unsigned count,
		enum subsys_notif_type notif, void *data)
{

	while (count--) {
		struct subsys_device *dev = *list++;
		struct notif_data notif_data;
		struct platform_device *pdev;

		if (!dev)
			continue;

		pdev = container_of(dev->desc->dev, struct platform_device,
									dev);
		dev->notif_state = notif;

		notif_data.crashed = subsys_get_crash_status(dev);
		notif_data.enable_ramdump = is_ramdump_enabled(dev);
		notif_data.enable_mini_ramdumps = enable_mini_ramdumps;
		notif_data.no_auth = dev->desc->no_auth;
		notif_data.pdev = pdev;

		subsys_notif_queue_notification(dev->notify, notif,
								&notif_data);
	}
}

int wait_for_shutdown_ack(struct subsys_desc *desc)
{
	int count;

	if (!desc || !desc->shutdown_ack_gpio)
		return 0;

	for (count = SHUTDOWN_ACK_MAX_LOOPS; count > 0; count--) {
		if (gpio_get_value(desc->shutdown_ack_gpio))
			return count;
		msleep(SHUTDOWN_ACK_DELAY_MS);
	}

	pr_err("[%s]: Timed out waiting for shutdown ack\n", desc->name);

	return -ETIMEDOUT;
}
EXPORT_SYMBOL(wait_for_shutdown_ack);

static void subsystem_shutdown(struct subsys_device *dev, void *data)
{
	const char *name = dev->desc->name;

	pr_info("[%s:%d]: Shutting down %s\n",
			current->comm, current->pid, name);
	if (dev->desc->shutdown(dev->desc, true) < 0)
		panic("subsys-restart: [%s:%d]: Failed to shutdown %s!",
			current->comm, current->pid, name);
	dev->crash_count++;
	subsys_set_state(dev, SUBSYS_OFFLINE);
}

static void subsystem_ramdump(struct subsys_device *dev, void *data)
{
	const char *name = dev->desc->name;
	if (dev->desc->ramdump)
		if (dev->desc->ramdump(is_ramdump_enabled(dev), dev->desc) < 0)
			pr_warn("%s[%s:%d]: Ramdump failed.\n",
				name, current->comm, current->pid);

	dev->do_ramdump_on_put = false;
}

static void subsystem_free_memory(struct subsys_device *dev, void *data)
{
	if (dev->desc->free_memory)
		dev->desc->free_memory(dev->desc);
}

static void subsystem_powerup(struct subsys_device *dev, void *data)
{
	const char *name = dev->desc->name;

	pr_info("[%s:%d]: Powering up %s\n", current->comm, current->pid, name);

	if (dev->desc->powerup(dev->desc) < 0) {
		notify_each_subsys_device(&dev, 1, SUBSYS_POWERUP_FAILURE,
								NULL);
		pr_emerg("[%s:%d]: Powerup error: %s!",
			current->comm, current->pid, name);
		return;
	}

	subsys_set_state(dev, SUBSYS_ONLINE);
	subsys_set_crash_status(dev, false);
}

static int __find_subsys(struct device *dev, void *data)
{
	struct subsys_device *subsys = to_subsys(dev);
	return !strcmp(subsys->desc->name, data);
}

static struct subsys_device *find_subsys(const char *str)
{
	struct device *dev;

	if (!str)
		return NULL;

	dev = bus_find_device(&subsys_bus_type, NULL, (void *)str,
			__find_subsys);
	return dev ? to_subsys(dev) : NULL;
}

static int subsys_start(struct subsys_device *subsys)
{
	int ret;

	notify_each_subsys_device(&subsys, 1, SUBSYS_BEFORE_POWERUP,
								NULL);

	ret = subsys->desc->powerup(subsys->desc);
	if (ret) {
		notify_each_subsys_device(&subsys, 1, SUBSYS_POWERUP_FAILURE,
									NULL);
		subsys->desc->shutdown(subsys->desc, false);
		return ret;
	}

	subsys_set_state(subsys, SUBSYS_ONLINE);

	if (subsys->desc->is_not_loadable)
		return 0;

	notify_each_subsys_device(&subsys, 1, SUBSYS_AFTER_POWERUP,
								NULL);
	return ret;
}

static void subsys_stop(struct subsys_device *subsys)
{
	notify_each_subsys_device(&subsys, 1, SUBSYS_BEFORE_SHUTDOWN, NULL);
	subsys->desc->shutdown(subsys->desc, false);
	subsys_set_state(subsys, SUBSYS_OFFLINE);
	notify_each_subsys_device(&subsys, 1, SUBSYS_AFTER_SHUTDOWN, NULL);
}

void *__subsystem_get(const char *name, const char *fw_name)
{
	struct subsys_device *subsys;
	struct subsys_device *subsys_d;
	int ret;
	void *retval;
	struct subsys_tracking *track;

	if (!name)
		return NULL;

	subsys = retval = find_subsys(name);
	if (!subsys)
		return ERR_PTR(-ENODEV);
	if (!try_module_get(subsys->owner)) {
		retval = ERR_PTR(-ENODEV);
		goto err_module;
	}

	/*parent must start before child*/
	if (subsys->desc->parent) {
		void *tmp = subsystem_get(subsys->desc->parent->name);

		if (IS_ERR_OR_NULL(tmp)) {
			pr_err("can't get %s subsystem\n",
					subsys->desc->parent->name);
			return tmp;
		}
	}

	subsys_d = subsystem_get(subsys->desc->depends_on);
	if (IS_ERR(subsys_d)) {
		retval = subsys_d;
		goto err_depends;
	}

	track = subsys_get_track(subsys);
	mutex_lock(&track->lock);
	if (!subsys->count) {
		if (fw_name) {
			pr_info("Changing subsys fw_name to %s\n", fw_name);
			strlcpy(subsys->desc->fw_name, fw_name,
				sizeof(subsys->desc->fw_name));
		}
		ret = subsys_start(subsys);
		if (ret) {
			retval = ERR_PTR(ret);
			goto err_start;
		}
	}
	subsys->count++;
	mutex_unlock(&track->lock);
	return retval;
err_start:
	mutex_unlock(&track->lock);
	subsystem_put(subsys_d);
err_depends:
	module_put(subsys->owner);
err_module:
	put_device(&subsys->dev);
	return retval;
}

/**
 * subsytem_get() - Boot a subsystem
 * @name: pointer to a string containing the name of the subsystem to boot
 *
 * This function returns a pointer if it succeeds. If an error occurs an
 * ERR_PTR is returned.
 *
 * If this feature is disable, the value %NULL will be returned.
 */
void *subsystem_get(const char *name)
{
	return __subsystem_get(name, NULL);
}
EXPORT_SYMBOL(subsystem_get);

/**
 * subsystem_get_with_fwname() - Boot a subsystem using the firmware name passed in
 * @name: pointer to a string containing the name of the subsystem to boot
 * @fw_name: pointer to a string containing the subsystem firmware image name
 *
 * This function returns a pointer if it succeeds. If an error occurs an
 * ERR_PTR is returned.
 *
 * If this feature is disable, the value %NULL will be returned.
 */
void *subsystem_get_with_fwname(const char *name, const char *fw_name)
{
	return __subsystem_get(name, fw_name);
}
EXPORT_SYMBOL(subsystem_get_with_fwname);

/**
 * subsystem_put() - Shutdown a subsystem
 * @peripheral_handle: pointer from a previous call to subsystem_get()
 *
 * This doesn't imply that a subsystem is shutdown until all callers of
 * subsystem_get() have called subsystem_put().
 */
void subsystem_put(void *subsystem)
{
	struct subsys_device *subsys_d, *subsys = subsystem;
	struct subsys_tracking *track;

	if (IS_ERR_OR_NULL(subsys))
		return;

	track = subsys_get_track(subsys);
	mutex_lock(&track->lock);
	if (WARN(!subsys->count, "%s: %s: Reference count mismatch\n",
			subsys->desc->name, __func__))
		goto err_out;
	if (!--subsys->count) {
		subsys_stop(subsys);
		if (subsys->do_ramdump_on_put)
			subsystem_ramdump(subsys, NULL);
		subsystem_free_memory(subsys, NULL);
	}
	mutex_unlock(&track->lock);

	subsys_d = find_subsys(subsys->desc->depends_on);
	if (subsys_d) {
		subsystem_put(subsys_d);
		put_device(&subsys_d->dev);
	}

	if (subsys->desc->parent) {
		struct subsys_device *subsys_p =
			desc_to_subsys(subsys->desc->parent->dev);
		subsystem_put(subsys_p);
	}

	module_put(subsys->owner);
	put_device(&subsys->dev);
	return;
err_out:
	mutex_unlock(&track->lock);
}
EXPORT_SYMBOL(subsystem_put);

static void subsystem_restart_wq_func(struct work_struct *work)
{
	struct subsys_device *dev = container_of(work,
						struct subsys_device, work);
	struct subsys_device **list;
	struct subsys_desc *desc = dev->desc;
	struct subsys_soc_restart_order *order = dev->restart_order;
	struct subsys_tracking *track;
	unsigned count;
	unsigned long flags;

	/*
	 * It's OK to not take the registration lock at this point.
	 * This is because the subsystem list inside the relevant
	 * restart order is not being traversed.
	 */
	if (order) {
		list = order->subsys_ptrs;
		count = order->count;
		track = &order->track;
	} else {
		list = &dev;
		count = 1;
		track = &dev->track;
	}

	mutex_lock(&track->lock);
	do_epoch_check(dev);

	if (dev->track.state == SUBSYS_OFFLINE) {
		mutex_unlock(&track->lock);
		WARN(1, "SSR aborted: %s subsystem not online\n", desc->name);
		return;
	}

	/*
	 * It's necessary to take the registration lock because the subsystem
	 * list in the SoC restart order will be traversed and it shouldn't be
	 * changed until _this_ restart sequence completes.
	 */
	mutex_lock(&soc_order_reg_lock);

	pr_debug("[%s:%d]: Starting restart sequence for %s\n",
			current->comm, current->pid, desc->name);
	notify_each_subsys_device(list, count, SUBSYS_BEFORE_SHUTDOWN, NULL);
	for_each_subsys_device(list, count, NULL, subsystem_shutdown);
	notify_each_subsys_device(list, count, SUBSYS_AFTER_SHUTDOWN, NULL);

	notify_each_subsys_device(list, count, SUBSYS_RAMDUMP_NOTIFICATION,
									NULL);

	spin_lock_irqsave(&track->s_lock, flags);
	track->p_state = SUBSYS_RESTARTING;
	spin_unlock_irqrestore(&track->s_lock, flags);

	/* Collect ram dumps for all subsystems in order here */
	for_each_subsys_device(list, count, NULL, subsystem_ramdump);

	for_each_subsys_device(list, count, NULL, subsystem_free_memory);

	notify_each_subsys_device(list, count, SUBSYS_BEFORE_POWERUP, NULL);
	for_each_subsys_device(list, count, NULL, subsystem_powerup);

	if (dev->track.state != SUBSYS_ONLINE) {
		pr_info("[%s:%d]: Restart sequence for %s failed.\n",
			current->comm, current->pid, desc->name);
	} else {
		notify_each_subsys_device(list, count, SUBSYS_AFTER_POWERUP,
					NULL);
		pr_info("[%s:%d]: Restart sequence for %s completed.\n",
			current->comm, current->pid, desc->name);
	}

	mutex_unlock(&soc_order_reg_lock);
	mutex_unlock(&track->lock);

	if (dev->track.state == SUBSYS_ONLINE) {
		spin_lock_irqsave(&track->s_lock, flags);
		track->p_state = SUBSYS_NORMAL;
		__pm_relax(&dev->ssr_wlock);
		spin_unlock_irqrestore(&track->s_lock, flags);
	}
}

static void __subsystem_restart_dev(struct subsys_device *dev)
{
	struct subsys_desc *desc = dev->desc;
	const char *name = dev->desc->name;
	struct subsys_tracking *track;
	unsigned long flags;
	struct subsys_device **list;
	struct subsys_soc_restart_order *order = dev->restart_order;
	unsigned int count;

	pr_debug("Restarting %s [level=%s]!\n", desc->name,
			restart_levels[dev->restart_level]);

	track = subsys_get_track(dev);
	if (order) {
		list = order->subsys_ptrs;
		count = order->count;
	} else {
		list = &dev;
		count = 1;
	}

	/*
	 * Allow drivers to call subsystem_restart{_dev}() as many times as
	 * they want up until the point where the subsystem is shutdown.
	 */
	spin_lock_irqsave(&track->s_lock, flags);
	if (track->p_state != SUBSYS_CRASHED &&
					dev->track.state == SUBSYS_ONLINE) {
		if (track->p_state != SUBSYS_RESTARTING) {
			track->p_state = SUBSYS_CRASHED;

			notify_each_subsys_device(list, count,
				SUBSYS_PREPARE_FOR_FATAL_SHUTDOWN, NULL);
			__pm_stay_awake(&dev->ssr_wlock);
			queue_work(ssr_wq, &dev->work);
		} else {
			panic("Subsystem %s crashed during SSR!", name);
		}
	} else
		WARN(dev->track.state == SUBSYS_OFFLINE,
			"SSR aborted: %s subsystem not online\n", name);
	spin_unlock_irqrestore(&track->s_lock, flags);
}

static void device_restart_work_hdlr(struct work_struct *work)
{
	struct subsys_device *dev = container_of(work, struct subsys_device,
							device_restart_work);

	notify_each_subsys_device(&dev, 1, SUBSYS_SOC_RESET, NULL);
	/*
	 * Temporary workaround until ramdump userspace application calls
	 * sync() and fclose() on attempting the dump.
	 */
	msleep(100);
	panic("subsys-restart: Resetting the SoC - %s crashed.",
							dev->desc->name);
}

int subsystem_restart_dev(struct subsys_device *dev)
{
	const char *name;

	if (!get_device(&dev->dev))
		return -ENODEV;

	if (!try_module_get(dev->owner)) {
		put_device(&dev->dev);
		return -ENODEV;
	}

	name = dev->desc->name;

	/*
	 * If a system reboot/shutdown is underway, ignore subsystem errors.
	 * However, print a message so that we know that a subsystem behaved
	 * unexpectedly here.
	 */
	if (system_state == SYSTEM_RESTART
		|| system_state == SYSTEM_POWER_OFF) {
		pr_err("%s crashed during a system poweroff/shutdown.\n", name);
		return -EBUSY;
	}

	pr_info("Restart sequence requested for %s, restart_level = %s.\n",
		name, restart_levels[dev->restart_level]);

	if (disable_restart_work == DISABLE_SSR) {
		pr_warn("subsys-restart: Ignoring restart request for %s.\n",
									name);
		return 0;
	}

	switch (dev->restart_level) {

	case RESET_SUBSYS_COUPLED:
		__subsystem_restart_dev(dev);
		break;
	case RESET_SOC:
		__pm_stay_awake(&dev->ssr_wlock);
		schedule_work(&dev->device_restart_work);
		return 0;
	default:
		panic("subsys-restart: Unknown restart level!\n");
		break;
	}
	module_put(dev->owner);
	put_device(&dev->dev);

	return 0;
}
EXPORT_SYMBOL(subsystem_restart_dev);

int subsystem_restart(const char *name)
{
	int ret;
	struct subsys_device *dev = find_subsys(name);

	if (!dev)
		return -ENODEV;

	ret = subsystem_restart_dev(dev);
	put_device(&dev->dev);
	return ret;
}
EXPORT_SYMBOL(subsystem_restart);

int subsystem_crashed(const char *name)
{
	struct subsys_device *dev = find_subsys(name);
	struct subsys_tracking *track;

	if (!dev)
		return -ENODEV;

	if (!get_device(&dev->dev))
		return -ENODEV;

	track = subsys_get_track(dev);

	mutex_lock(&track->lock);
	dev->do_ramdump_on_put = true;
	/*
	 * TODO: Make this work with multiple consumers where one is calling
	 * subsystem_restart() and another is calling this function. To do
	 * so would require updating private state, etc.
	 */
	mutex_unlock(&track->lock);

	put_device(&dev->dev);
	return 0;
}
EXPORT_SYMBOL(subsystem_crashed);

void subsys_set_crash_status(struct subsys_device *dev, bool crashed)
{
	dev->crashed = crashed;
}

bool subsys_get_crash_status(struct subsys_device *dev)
{
	return dev->crashed;
}

static struct subsys_device *desc_to_subsys(struct device *d)
{
	struct subsys_device *device, *subsys_dev = 0;

	mutex_lock(&subsys_list_lock);
	list_for_each_entry(device, &subsys_list, list)
		if (device->desc->dev == d)
			subsys_dev = device;
	mutex_unlock(&subsys_list_lock);
	return subsys_dev;
}

void notify_proxy_vote(struct device *device)
{
	struct subsys_device *dev = desc_to_subsys(device);

	if (dev)
		notify_each_subsys_device(&dev, 1, SUBSYS_PROXY_VOTE, NULL);
}

void notify_proxy_unvote(struct device *device)
{
	struct subsys_device *dev = desc_to_subsys(device);

	if (dev)
		notify_each_subsys_device(&dev, 1, SUBSYS_PROXY_UNVOTE, NULL);
}

static int subsys_device_open(struct inode *inode, struct file *file)
{
	struct subsys_device *device, *subsys_dev = 0;
	void *retval;

	mutex_lock(&subsys_list_lock);
	list_for_each_entry(device, &subsys_list, list)
		if (MINOR(device->dev_no) == iminor(inode))
			subsys_dev = device;
	mutex_unlock(&subsys_list_lock);

	if (!subsys_dev)
		return -EINVAL;

	retval = subsystem_get_with_fwname(subsys_dev->desc->name,
					subsys_dev->desc->fw_name);
	if (IS_ERR(retval))
		return PTR_ERR(retval);

	return 0;
}

static int subsys_device_close(struct inode *inode, struct file *file)
{
	struct subsys_device *device, *subsys_dev = 0;

	mutex_lock(&subsys_list_lock);
	list_for_each_entry(device, &subsys_list, list)
		if (MINOR(device->dev_no) == iminor(inode))
			subsys_dev = device;
	mutex_unlock(&subsys_list_lock);

	if (!subsys_dev)
		return -EINVAL;

	subsystem_put(subsys_dev);
	return 0;
}

static const struct file_operations subsys_device_fops = {
		.owner = THIS_MODULE,
		.open = subsys_device_open,
		.release = subsys_device_close,
};

static void subsys_device_release(struct device *dev)
{
	struct subsys_device *subsys = to_subsys(dev);

	wakeup_source_trash(&subsys->ssr_wlock);
	mutex_destroy(&subsys->track.lock);
	ida_simple_remove(&subsys_ida, subsys->id);
	kfree(subsys);
}

static int subsys_char_device_add(struct subsys_device *subsys_dev)
{
	int ret = 0;
	static int major, minor;
	dev_t dev_no;

	mutex_lock(&char_device_lock);
	if (!major) {
		ret = alloc_chrdev_region(&dev_no, 0, 4, "subsys");
		if (ret < 0) {
			pr_err("Failed to alloc subsys_dev region, err %d\n",
									ret);
			goto fail;
		}
		major = MAJOR(dev_no);
		minor = MINOR(dev_no);
	} else
		dev_no = MKDEV(major, minor);

	if (!device_create(char_class, subsys_dev->desc->dev, dev_no,
			NULL, "subsys_%s", subsys_dev->desc->name)) {
		pr_err("Failed to create subsys_%s device\n",
						subsys_dev->desc->name);
		goto fail_unregister_cdev_region;
	}

	cdev_init(&subsys_dev->char_dev, &subsys_device_fops);
	subsys_dev->char_dev.owner = THIS_MODULE;
	ret = cdev_add(&subsys_dev->char_dev, dev_no, 1);
	if (ret < 0)
		goto fail_destroy_device;

	subsys_dev->dev_no = dev_no;
	minor++;
	mutex_unlock(&char_device_lock);

	return 0;

fail_destroy_device:
	device_destroy(char_class, dev_no);
fail_unregister_cdev_region:
	unregister_chrdev_region(dev_no, 1);
fail:
	mutex_unlock(&char_device_lock);
	return ret;
}

static void subsys_char_device_remove(struct subsys_device *subsys_dev)
{
	cdev_del(&subsys_dev->char_dev);
	device_destroy(char_class, subsys_dev->dev_no);
	unregister_chrdev_region(subsys_dev->dev_no, 1);
}

static void subsys_remove_restart_order(struct device_node *device)
{
	struct subsys_soc_restart_order *order;
	int i;

	mutex_lock(&ssr_order_mutex);
	list_for_each_entry(order, &ssr_order_list, list)
		for (i = 0; i < order->count; i++)
			if (order->device_ptrs[i] == device)
				order->subsys_ptrs[i] = NULL;
	mutex_unlock(&ssr_order_mutex);
}

static struct subsys_soc_restart_order *ssr_parse_restart_orders(struct
							subsys_desc * desc)
{
	int i, j, count, num = 0;
	struct subsys_soc_restart_order *order, *tmp;
	struct device *dev = desc->dev;
	struct device_node *ssr_node;
	uint32_t len;

	if (!of_get_property(dev->of_node, "qcom,restart-group", &len))
		return NULL;

	count = len/sizeof(uint32_t);

	order = devm_kzalloc(dev, sizeof(*order), GFP_KERNEL);
	if (!order)
		return ERR_PTR(-ENOMEM);

	order->subsys_ptrs = devm_kzalloc(dev,
				count * sizeof(struct subsys_device *),
				GFP_KERNEL);
	if (!order->subsys_ptrs)
		return ERR_PTR(-ENOMEM);

	order->device_ptrs = devm_kzalloc(dev,
				count * sizeof(struct device_node *),
				GFP_KERNEL);
	if (!order->device_ptrs)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < count; i++) {
		ssr_node = of_parse_phandle(dev->of_node,
						"qcom,restart-group", i);
		if (!ssr_node)
			return ERR_PTR(-ENXIO);
		of_node_put(ssr_node);
		pr_info("%s device has been added to %s's restart group\n",
						ssr_node->name, desc->name);
		order->device_ptrs[i] = ssr_node;
	}

	/*
	 * Check for similar restart groups. If found, return
	 * without adding the new group to the ssr_order_list.
	 */
	mutex_lock(&ssr_order_mutex);
	list_for_each_entry(tmp, &ssr_order_list, list) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < count; j++) {
				if (order->device_ptrs[j] !=
					tmp->device_ptrs[i])
					continue;
				else
					num++;
			}
		}

		if (num == count && tmp->count == count)
			goto err;
		else if (num) {
			tmp = ERR_PTR(-EINVAL);
			goto err;
		}
	}

	order->count = count;
	mutex_init(&order->track.lock);
	spin_lock_init(&order->track.s_lock);

	INIT_LIST_HEAD(&order->list);
	list_add_tail(&order->list, &ssr_order_list);
	mutex_unlock(&ssr_order_mutex);

	return order;
err:
	mutex_unlock(&ssr_order_mutex);
	return tmp;
}

static int __get_gpio(struct subsys_desc *desc, const char *prop,
		int *gpio)
{
	struct device_node *dnode = desc->dev->of_node;
	int ret = -ENOENT;

	if (of_find_property(dnode, prop, NULL)) {
		*gpio = of_get_named_gpio(dnode, prop, 0);
		ret = *gpio < 0 ? *gpio : 0;
	}

	return ret;
}

static int __get_irq(struct subsys_desc *desc, const char *prop,
		unsigned int *irq, int *gpio)
{
	int ret, gpiol, irql;

	if (of_property_read_bool(desc->dev->of_node, "qca,extended-intc") ||
		of_property_read_bool(desc->dev->parent->of_node,
					"qca,extended-intc")) {
		struct platform_device *pdev = container_of(desc->dev,
				struct platform_device, dev);

		irql = platform_get_irq_byname(pdev, prop);
		if (irql < 0) {
			pr_err("[%s]: Error getting IRQ \"%s\"\n", desc->name,
				prop);
			return irql;
		}
		*irq = irql;
		return 0;
	}

	ret = __get_gpio(desc, prop, &gpiol);
	if (ret)
		return ret;

	irql = gpio_to_irq(gpiol);

	if (irql == -ENOENT)
		irql = -ENXIO;

	if (irql < 0) {
		pr_err("[%s]: Error getting IRQ \"%s\"\n", desc->name,
				prop);
		return irql;
	} else {
		if (gpio)
			*gpio = gpiol;
		*irq = irql;
	}

	return 0;
}

static int subsys_parse_devicetree(struct subsys_desc *desc)
{
	struct subsys_soc_restart_order *order;
	int ret;

	struct platform_device *pdev = container_of(desc->dev,
					struct platform_device, dev);

	ret = __get_irq(desc, "fatal", &desc->err_fatal_irq,
							&desc->err_fatal_gpio);
	if (ret && ret != -ENOENT)
		return ret;

	ret = __get_irq(desc, "stop-ack", &desc->stop_ack_irq, NULL);
	if (ret && ret != -ENOENT)
		return ret;

	ret = __get_gpio(desc, "qcom,gpio-force-stop", &desc->force_stop_gpio);
	if (ret && ret != -ENOENT)
		return ret;

	ret = __get_gpio(desc, "qcom,gpio-ramdump-disable",
			&desc->ramdump_disable_gpio);
	if (ret && ret != -ENOENT)
		return ret;

	ret = __get_gpio(desc, "qcom,gpio-shutdown-ack",
			&desc->shutdown_ack_gpio);
	if (ret && ret != -ENOENT)
		return ret;

	if (!desc->parent) {
		if (of_property_read_bool(desc->dev->of_node,
						"qca,extended-intc"))
			ret = platform_get_irq_byname(pdev, "wdog");
		else
			/* There is only one irq */
			ret = platform_get_irq(pdev, 0);

		if (ret > 0)
			desc->wdog_bite_irq = ret;
	}

	order = ssr_parse_restart_orders(desc);
	if (IS_ERR(order)) {
		pr_err("Could not initialize SSR restart order, err = %ld\n",
							PTR_ERR(order));
		return PTR_ERR(order);
	}

	return 0;
}

static int subsys_setup_irqs(struct subsys_device *subsys)
{
	struct subsys_desc *desc = subsys->desc;
	int ret;
	char *err_fatal_int;
	char *stop_ack_int;

	if (desc->err_fatal_irq && desc->err_fatal_handler) {
		if (!desc->parent)
			err_fatal_int = "err_fatal_interrupt";
		else {
			err_fatal_int = devm_kzalloc(desc->dev, BUF_SIZE,
								GFP_KERNEL);
			if (!err_fatal_int)
				return -ENOMEM;
			strlcpy(err_fatal_int, desc->name, BUF_SIZE);
			strlcat(err_fatal_int, "_fatal", BUF_SIZE);
		}
		ret = devm_request_threaded_irq(desc->dev, desc->err_fatal_irq,
				NULL, desc->err_fatal_handler,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				err_fatal_int, desc);
		if (ret < 0) {
			dev_err(desc->dev, "[%s]: Unable to register error fatal IRQ handler!: %d\n",
				desc->name, ret);
			return ret;
		}
	}

	if (desc->stop_ack_irq && desc->stop_ack_handler) {
		if (!desc->parent)
			stop_ack_int = "stop_ack_interrupt";
		else {
			stop_ack_int = devm_kzalloc(desc->dev, BUF_SIZE,
								GFP_KERNEL);
			if (!stop_ack_int)
				return -ENOMEM;
			strlcpy(stop_ack_int, desc->name, BUF_SIZE);
			strlcat(stop_ack_int, "_stop_ack", BUF_SIZE);
		}
		ret = devm_request_threaded_irq(desc->dev, desc->stop_ack_irq,
				NULL, desc->stop_ack_handler,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				stop_ack_int, desc);
		if (ret < 0) {
			dev_err(desc->dev, "[%s]: Unable to register stop ack handler!: %d\n",
				desc->name, ret);
			return ret;
		}
	}

	if (desc->wdog_bite_irq && desc->wdog_bite_handler && !desc->parent) {
		ret = devm_request_threaded_irq(desc->dev, desc->wdog_bite_irq,
				NULL, desc->wdog_bite_handler,
				IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				"q6_wdog_interrupt", desc);
		if (ret < 0) {
			dev_err(desc->dev, "[%s]: Unable to register wdog bite handler!: %d\n",
				desc->name, ret);
			return ret;
		}
	}

	return 0;
}

static void subsys_free_irqs(struct subsys_device *subsys)
{
	struct subsys_desc *desc = subsys->desc;

	if (desc->err_fatal_irq && desc->err_fatal_handler)
		devm_free_irq(desc->dev, desc->err_fatal_irq, desc);
	if (desc->stop_ack_irq && desc->stop_ack_handler)
		devm_free_irq(desc->dev, desc->stop_ack_irq, desc);
	if (desc->wdog_bite_irq && desc->wdog_bite_handler)
		devm_free_irq(desc->dev, desc->wdog_bite_irq, desc);
}

void subsys_add_child(struct subsys_desc *desc, struct subsys_child *child)
{
	list_add(&child->node, &desc->child);
}
EXPORT_SYMBOL(subsys_add_child);

void subsys_remove_child(struct subsys_desc *desc, struct subsys_child *child)
{
	list_del(&child->node);
}
EXPORT_SYMBOL(subsys_remove_child);
struct subsys_device *subsys_register(struct subsys_desc *desc)
{
	struct subsys_device *subsys;
	struct device_node *ofnode = desc->dev->of_node;
	int ret;

	subsys = kzalloc(sizeof(*subsys), GFP_KERNEL);
	if (!subsys)
		return ERR_PTR(-ENOMEM);

	subsys->desc = desc;
	subsys->owner = desc->owner;
	subsys->dev.parent = desc->dev;
	subsys->dev.bus = &subsys_bus_type;
	subsys->dev.release = subsys_device_release;
	subsys->notif_state = -1;
	subsys->desc->sysmon_pid = -1;
	strlcpy(subsys->desc->fw_name, desc->name,
			sizeof(subsys->desc->fw_name));

	subsys->notify = subsys_notif_add_subsys(desc->name);

	snprintf(subsys->wlname, sizeof(subsys->wlname), "ssr(%s)", desc->name);
	wakeup_source_init(&subsys->ssr_wlock, subsys->wlname);
	INIT_WORK(&subsys->work, subsystem_restart_wq_func);
	INIT_WORK(&subsys->device_restart_work, device_restart_work_hdlr);
	spin_lock_init(&subsys->track.s_lock);

	subsys->id = ida_simple_get(&subsys_ida, 0, 0, GFP_KERNEL);
	if (subsys->id < 0) {
		wakeup_source_trash(&subsys->ssr_wlock);
		ret = subsys->id;
		kfree(subsys);
		return ERR_PTR(ret);
	}

	dev_set_name(&subsys->dev, "subsys%d", subsys->id);

	mutex_init(&subsys->track.lock);

	ret = device_register(&subsys->dev);
	if (ret) {
		put_device(&subsys->dev);
		return ERR_PTR(ret);
	}

	ret = subsys_char_device_add(subsys);
	if (ret) {
		goto err_register;
	}

	if (ofnode) {
		ret = subsys_parse_devicetree(desc);
		if (ret)
			goto err_register;

		subsys->restart_order = update_restart_order(subsys);

		ret = subsys_setup_irqs(subsys);
		if (ret < 0)
			goto err_setup_irqs;

		if (of_property_read_u32(ofnode, "qcom,ssctl-instance-id",
					&desc->ssctl_instance_id))
			pr_debug("Reading instance-id for %s failed\n",
								desc->name);

		subsys->desc->edge = of_get_property(ofnode, "qcom,edge",
									NULL);
		if (!subsys->desc->edge)
			pr_debug("Reading qcom,edge for %s failed\n",
								desc->name);
		if (of_property_read_bool(desc->dev->of_node,
					"qca,auto-restart"))
			subsys->restart_level = RESET_SUBSYS_COUPLED;
	}

	mutex_lock(&subsys_list_lock);
	INIT_LIST_HEAD(&subsys->list);
	list_add_tail(&subsys->list, &subsys_list);
	mutex_unlock(&subsys_list_lock);
	INIT_LIST_HEAD(&subsys->desc->child);

	return subsys;

err_setup_irqs:
	if (ofnode)
		subsys_remove_restart_order(ofnode);
err_register:
	device_unregister(&subsys->dev);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL(subsys_register);

void subsys_unregister(struct subsys_device *subsys)
{
	struct subsys_device *subsys_dev, *tmp;
	struct device_node *device = subsys->desc->dev->of_node;

	if (IS_ERR_OR_NULL(subsys))
		return;

	if (get_device(&subsys->dev)) {
		mutex_lock(&subsys_list_lock);
		list_for_each_entry_safe(subsys_dev, tmp, &subsys_list, list)
			if (subsys_dev == subsys)
				list_del(&subsys->list);
		mutex_unlock(&subsys_list_lock);

		if (device) {
			subsys_free_irqs(subsys);
			subsys_remove_restart_order(device);
		}
		mutex_lock(&subsys->track.lock);
		WARN_ON(subsys->count);
		device_unregister(&subsys->dev);
		mutex_unlock(&subsys->track.lock);
		subsys_char_device_remove(subsys);
		put_device(&subsys->dev);
	}
}
EXPORT_SYMBOL(subsys_unregister);

static int subsys_panic(struct device *dev, void *data)
{
	struct subsys_device *subsys = to_subsys(dev);

	if (subsys->desc->crash_shutdown)
		subsys->desc->crash_shutdown(subsys->desc);
	return 0;
}

static int ssr_panic_handler(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	bus_for_each_dev(&subsys_bus_type, NULL, NULL, subsys_panic);
	return NOTIFY_DONE;
}

static struct notifier_block panic_nb = {
	.notifier_call  = ssr_panic_handler,
	.priority = 100,
};

static int __init subsys_restart_init(void)
{
	int ret;

	ssr_wq = alloc_workqueue("ssr_wq", WQ_CPU_INTENSIVE, 0);
	BUG_ON(!ssr_wq);

	ret = bus_register(&subsys_bus_type);
	if (ret)
		goto err_bus;

	char_class = class_create(THIS_MODULE, "subsys");
	if (IS_ERR(char_class)) {
		ret = -ENOMEM;
		pr_err("Failed to create subsys_dev class\n");
		goto err_class;
	}

	ret = atomic_notifier_chain_register(&panic_notifier_list,
			&panic_nb);
	if (ret)
		goto err_soc;

	return 0;

err_soc:
	class_destroy(char_class);
err_class:
	bus_unregister(&subsys_bus_type);
err_bus:
	destroy_workqueue(ssr_wq);
	return ret;
}
arch_initcall(subsys_restart_init);

MODULE_DESCRIPTION("Subsystem Restart Driver");
MODULE_LICENSE("GPL v2");
