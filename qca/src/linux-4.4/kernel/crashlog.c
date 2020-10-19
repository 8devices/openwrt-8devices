/*
 * Crash information logger
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 * Based on ramoops.c
 *   Copyright (C) 2010 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <linux/debugfs.h>
#include <linux/crashlog.h>
#include <linux/kmsg_dump.h>
#include <linux/module.h>
#include <linux/pfn.h>
#include <asm/io.h>

#define CRASHLOG_PAGES	4
#define CRASHLOG_SIZE	(CRASHLOG_PAGES * PAGE_SIZE)
#define CRASHLOG_MAGIC	0xa1eedead

/*
 * Start the log at 1M before the end of RAM, as some boot loaders like
 * to use the end of the RAM for stack usage and other things
 * If this fails, fall back to using the last part.
 */
#define CRASHLOG_OFFSET	(1024 * 1024)

struct crashlog_data {
	u32 magic;
	u32 len;
	u8 data[];
};

static struct debugfs_blob_wrapper crashlog_blob;
static unsigned long crashlog_addr = 0;
static struct crashlog_data *crashlog_buf;
static struct kmsg_dumper dump;
static bool first = true;

extern struct list_head *crashlog_modules;

#ifndef CONFIG_NO_BOOTMEM
void __init crashlog_init_bootmem(bootmem_data_t *bdata)
{
	unsigned long addr;

	if (crashlog_addr)
		return;

	addr = PFN_PHYS(bdata->node_low_pfn) - CRASHLOG_OFFSET;
	if (reserve_bootmem(addr, CRASHLOG_SIZE, BOOTMEM_EXCLUSIVE) < 0) {
		printk("Crashlog failed to allocate RAM at address 0x%lx\n", addr);
		bdata->node_low_pfn -= CRASHLOG_PAGES;
		addr = PFN_PHYS(bdata->node_low_pfn);
	}
	crashlog_addr = addr;
}
#endif

#ifdef CONFIG_HAVE_MEMBLOCK
void __meminit crashlog_init_memblock(phys_addr_t addr, phys_addr_t size)
{
	if (crashlog_addr)
		return;

	addr += size - CRASHLOG_OFFSET;
	if (memblock_reserve(addr, CRASHLOG_SIZE)) {
		printk("Crashlog failed to allocate RAM at address 0x%lx\n", (unsigned long) addr);
		return;
	}

	crashlog_addr = addr;
}
#endif

static void __init crashlog_copy(void)
{
	if (crashlog_buf->magic != CRASHLOG_MAGIC)
		return;

	if (!crashlog_buf->len || crashlog_buf->len >
	    CRASHLOG_SIZE - sizeof(*crashlog_buf))
		return;

	crashlog_blob.size = crashlog_buf->len;
	crashlog_blob.data = kmemdup(crashlog_buf->data,
		crashlog_buf->len, GFP_KERNEL);

	debugfs_create_blob("crashlog", 0700, NULL, &crashlog_blob);
}

static int get_maxlen(void)
{
	return CRASHLOG_SIZE - sizeof(*crashlog_buf) - crashlog_buf->len;
}

static void crashlog_printf(const char *fmt, ...)
{
	va_list args;
	int len = get_maxlen();

	if (!len)
		return;

	va_start(args, fmt);
	crashlog_buf->len += vscnprintf(
		&crashlog_buf->data[crashlog_buf->len],
		len, fmt, args);
	va_end(args);
}

static void crashlog_do_dump(struct kmsg_dumper *dumper,
		enum kmsg_dump_reason reason)
{
	struct timeval tv;
	struct module *m;
	char *buf;
	size_t len;

	if (!first)
		crashlog_printf("\n===================================\n");

	do_gettimeofday(&tv);
	crashlog_printf("Time: %lu.%lu\n",
		(long)tv.tv_sec, (long)tv.tv_usec);

	if (first) {
		crashlog_printf("Modules:");
		list_for_each_entry(m, crashlog_modules, list) {
			crashlog_printf("\t%s@%p+%x", m->name,
			m->module_core, m->core_size,
			m->module_init, m->init_size);
		}
		crashlog_printf("\n");
		first = false;
	}

	buf = (char *)&crashlog_buf->data[crashlog_buf->len];

	kmsg_dump_get_buffer(dumper, true, buf, get_maxlen(), &len);

	crashlog_buf->len += len;
}


int __init crashlog_init_fs(void)
{
	if (!crashlog_addr)
		return -ENOMEM;

	crashlog_buf = ioremap(crashlog_addr, CRASHLOG_SIZE);

	crashlog_copy();

	crashlog_buf->magic = CRASHLOG_MAGIC;
	crashlog_buf->len = 0;

	dump.max_reason = KMSG_DUMP_OOPS;
	dump.dump = crashlog_do_dump;
	kmsg_dump_register(&dump);

	return 0;
}
module_init(crashlog_init_fs);
