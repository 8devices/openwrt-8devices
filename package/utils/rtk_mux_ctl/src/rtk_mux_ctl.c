/*
 * rtk_mux_ctl - Realtek SoC pinmux helper
 *
 *   Copyright (C) 2019 Rytis Zigmantavicius <rytis.z@8devices.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <regex.h>

#include <jansson.h>

#include <fcntl.h>
#include <sys/mman.h>

#include "rtk_data.h"

#define DEFAULT_CONFIG_LOCATION "/etc/rtk_mux_conf.json"

#define MMAP_PATH "/dev/mem"

#ifdef DEBUG
#define debug_print(fmt, args...)  fprintf(stderr, "DBG: %s (%s +%d): " fmt, \
					   __func__, __FILE__, __LINE__, ##args)
#else
#define debug_print(fmt, args...)
#endif

struct mux_config {
	struct rtl_gpio * gpio;
	int func_id;
	int gpio_dir;
	int gpio_val;
};

struct config_info {
	struct mux_config *muxes;
	int mux_count;
	struct device_in_out_func *dev_info;
};

extern uint32_t getpagesize();
int mmap_setup(void **pmmap, size_t size, off_t base)
{
	uint8_t *mmap_reg;
	int mmap_fd;
	uint32_t pagesize = getpagesize();
	uint32_t block_base_offset;
	uint16_t offset_from_block;

	if ((mmap_fd = open(MMAP_PATH, O_RDWR)) < 0) {
		printf("mmap: unable to open %s\n", MMAP_PATH);
		return -1;
	}

	block_base_offset = base & ~(pagesize - 1);
	offset_from_block = base & (pagesize - 1);

	mmap_reg = mmap(NULL, size + offset_from_block, PROT_READ | PROT_WRITE,
			MAP_FILE | MAP_SHARED, mmap_fd, block_base_offset);

	if (mmap_reg == MAP_FAILED) {
		close(mmap_fd);
		printf("mmap registers failed\n");
		return -1;
	}
	*pmmap = (void *)mmap_reg + offset_from_block;

	close(mmap_fd);

	return 0;
}

void mmap_close(void *mmap_ptr, uint32_t size)
{
	uint32_t block_start;
	uint16_t start_offset;
	uint32_t pagesize = getpagesize();

	block_start = (uint32_t) mmap_ptr & ~(pagesize - 1);
	start_offset = (uint32_t) mmap_ptr & (pagesize - 1);

	munmap((void *)block_start, size + start_offset);
}

void mmap_read_regs(int reg_size, void *mmap_ptr, uint32_t ** regs)
{
	int i;

	for (i = 0; i < reg_size; i += 4)
	{
		(*regs)[i / 4] = *((volatile uint32_t *)(mmap_ptr + i));
	}
}

void mmap_write_regs(int reg_size, void *mmap_ptr, uint32_t * regs)
{
	int i;

	for (i = 0; i < reg_size; i += 4) {
		*((volatile uint32_t *)(mmap_ptr + i)) = regs[i / 4];
	}
}

uint32_t **get_regs(struct device_in_out_func *info)
{
	void *pmmap;
	uint32_t **regs;
	int size;
	int i;

	regs  = calloc(info->reg_mappings_count, sizeof(uint32_t));
	for (i = 0; i < info->reg_mappings_count; i++)
	{
		size = info->regs[i].reg_count * sizeof(uint32_t);
		regs[i] = calloc(info->regs[i].reg_count, sizeof(uint32_t));

		if (!regs)
			return NULL;

		if (mmap_setup(&pmmap, size, info->regs[i].offset))
			return NULL;

		mmap_read_regs(size, pmmap, &regs[i]);
		mmap_close(pmmap, size);
	}

	return regs;
}

int set_regs(uint32_t ** regs, struct device_in_out_func *info)
{
	void *pmmap;
	int size;
	int i;

	for (i = 0; i < info->reg_mappings_count; i++)
	{
		size = info->regs[i].reg_count * sizeof(uint32_t);
		if (mmap_setup(&pmmap, size, info->regs[i].offset))
			return -1;

		mmap_write_regs(size, pmmap, regs[i]);
		mmap_close(pmmap, size);
	}

	return 0;
}

void dump_regs(uint32_t ** regs, struct device_in_out_func *info)
{
	int i,j;

	for (i = 0; i < info->reg_mappings_count; i++) {
		printf("%s REG:\n", info->regs[i].name);
		for (j = 0; j < info->regs[i].reg_count; j++) {
			printf("0x%02x: 0x%08x\n", j * 4, regs[i][j]);
		}
		printf("\n");
	}
}

int get_gpio_func_id(uint32_t reg, uint8_t shift, uint32_t * regs)
{
	uint32_t word = reg / 4;

	return (regs[word] >> shift) & 0xf;
}

int set_gpio_func_id(int func_id, uint32_t reg, uint8_t shift, uint32_t * regs)
{
	uint32_t word = reg / 4;

	if (func_id < 0)
		return -1;

	regs[word] &= ~(0xf << shift);
	regs[word] |= func_id << shift;

	return 0;
}

int parse_gpio_func(struct rtl_gpio *gpio, int func_id, int dir, int val, char *output)
{
	const char *func_name;

	func_name = gpio->func_array[func_id];

	if (func_id >= 0 && func_id < gpio->func_count) {
		strcpy(output, func_name);
		if (!strcmp(func_name, "GPIO")) {
			if (dir >= 0)
				sprintf(output + strlen(output), " %s", dir ? "OUT" : "IN");
			if (val >= 0)
				sprintf(output + strlen(output), " %s", val ? "HIGH" : "LOW");
		}
		return 0;
	}
	strcpy(output, "UNKNOWN");

	return -1;
}

void parse_current_gpio_func(struct rtl_gpio *gpio, uint32_t **regs, char *func_name,
			     struct device_in_out_func *info)
{
	int func_id;
	int gpio_dir;
	int gpio_val;


	func_id = get_gpio_func_id(gpio->reg, gpio->shift, regs[0]);

	gpio_dir = info->get_gpio_dir(regs[1], gpio->gpio);
	gpio_val = info->get_gpio_val(regs[1], gpio->gpio);

	parse_gpio_func(gpio, func_id, gpio_dir, gpio_val, func_name);
}

void dump_current_config(uint32_t ** regs, struct device_in_out_func *info)
{
	char func_name[256];
	int i;

	for (i = 0; i < info->gpio_mux_count; i++) {
		parse_current_gpio_func(&info->gpio_mux[i], regs, func_name, info);
		printf("%s: %s\n", info->gpio_mux[i].name, func_name);
	}
}

void dump_config_changes(uint32_t ** regs, struct config_info *config)
{
	int i;
	char old_func[256];
	char new_func[256];

	struct mux_config *mux_cfg;
	struct device_in_out_func *dev_info = config->dev_info;

	for(i = 0; i < config->mux_count; i++) {
		mux_cfg = &config->muxes[i];

		parse_current_gpio_func(mux_cfg->gpio, regs, old_func, dev_info);
		parse_gpio_func(mux_cfg->gpio, mux_cfg->func_id,
				mux_cfg->gpio_dir, mux_cfg->gpio_val, new_func);

		if (strcmp(old_func, new_func))
			printf("%s: %s -> %s\n", mux_cfg->gpio->name, old_func, new_func);
	}
}

int check_function_overlay(struct config_info config, struct device_in_out_func *info)
{
	int i, j;

	for (i = 0; i < config.mux_count - 1; i++)
		for ( j = i + 1; j < config.mux_count; j++)
			if (config.muxes[i].gpio == config.muxes[j].gpio)
			{
				printf("error: multiple definitions of %s gpio",
				       config.muxes[i].gpio->name);
				return -1;
			}

	return 0;
}

int get_function_id(const char * mux, int func_count, const char **func_array)
{
	int i;

	for (i = 0; i < func_count; i++)
		if (!strcmp(func_array[i], mux))
			return i;

	return -1;
}


void print_available_muxes(struct rtl_gpio *gpio_mux)
{
	int i;

	printf("%s - ", gpio_mux->name);
	for (i = 0; i < gpio_mux->func_count; i++) {
		printf("%s ", gpio_mux->func_array[i]);
	}
	printf("\n");
}

void print_all_gpios(struct device_in_out_func *info)
{
	int i;

	for (i = 0; i < info->gpio_mux_count; i++) {
		print_available_muxes(&info->gpio_mux[i]);
	}
}

int load_config(char *config_file, struct config_info *config)
{
	int i;

	json_error_t error;
	json_t *root, *mux_list, *mux_el;
	json_t *js_obj;
	json_t *js_mux;

	struct rtl_gpio *gpio_info;
	struct mux_config *current_mux;
	struct device_in_out_func *info = config->dev_info;

	root = json_load_file(config_file, 0, &error);
	if (!json_is_object(root)) {
		printf("json error: on line %d: %s\n", error.line, error.text);
		json_decref(root);

		return -1;
	}

	mux_list = json_object_get(root, "mux_list");
	if (!json_is_array(mux_list)) {
		printf("json error: mux load error on line %d: %s\n", error.line, error.text);
		json_decref(root);

		return -1;
	}

	config->mux_count = json_array_size(mux_list);
	if (config->mux_count > 0) {
		config->muxes = calloc(config->mux_count, sizeof(struct mux_config));
	}

	for (i = 0; i < config->mux_count; i++) {
		mux_el = json_array_get(mux_list, i);
		current_mux = &config->muxes[i];

		js_obj = json_object_get(mux_el, "gpio");
		if (!json_is_string(js_obj)) {
			printf("json error: gpio is not a string\n");
			goto free_muxes;
		}

		gpio_info = info->get_gpio_by_name(json_string_value(js_obj));

		if (!gpio_info) {
			printf("\nconfig error: \"%s\" not found\n", json_string_value(js_obj));
			printf("List of available gpios:\n");
			print_all_gpios(info);
			goto free_muxes;
		}

		current_mux->gpio = gpio_info;

		js_mux = json_object_get(mux_el, "mux");

		if (!json_is_string(js_mux)) {
			printf("json error: mux is not a string\n");
			goto free_muxes;
		}

		current_mux->func_id = get_function_id(json_string_value(js_mux),
						       gpio_info->func_count, gpio_info->func_array);

		if (current_mux->func_id < 0) {
			printf("\nconfig error: \"%s\" fucntion is not availabe for %s\n", json_string_value(js_mux), gpio_info->name);
			printf("availabe functions:\n");
			print_available_muxes(gpio_info);
			goto free_muxes;
		}

		current_mux->gpio_dir = -1;
		current_mux->gpio_val = -1;

		js_obj = json_object_get(mux_el, "direction");
		if (js_obj) {
			if (!json_is_string(js_obj)) {
				printf("\njson error: GPIO direction is not a string\n");
				goto free_muxes;
			}

			if (strncmp(json_string_value(js_obj), "IN", 2) == 0)
				current_mux->gpio_dir = 0;
			else if (strncmp(json_string_value(js_obj), "OUT", 3) == 0)
				current_mux->gpio_dir = 1;
			else {
				printf("\nconfig error: gpio direction invalid\n");
				printf("availabe options: \"IN\" \"OUT\"\n");
				goto free_muxes;
			}
		}

		js_obj = json_object_get(mux_el, "value");
		if (js_obj) {
			if (!json_is_string(js_obj)) {
				printf("\njson error: GPIO direction is not a string\n");
				goto free_muxes;
			}

			if (strncmp(json_string_value(js_obj), "HIGH", 2) == 0)
				current_mux->gpio_val = 1;
			else if (strncmp(json_string_value(js_obj), "LOW", 3) == 0)
				current_mux->gpio_val = 0;
			else {
				printf("\nconfig error: gpio value invalid\n");
				printf("availabe options: \"HIGH\" \"LOW\"\n");
				goto free_muxes;
			}
		}
		debug_print("%s: dir = %d,  val = %d\n",
			    current_mux->gpio->name,
			    current_mux->gpio_dir,
			    current_mux->gpio_val);
	}

	json_decref(root);

	return 0;

free_muxes:
	json_decref(root);
	free(config->muxes);
	config->muxes = NULL;
	config->mux_count = 0;

	return -1;
}

int apply_config_changes(struct config_info *config, uint32_t **regs)
{
	int i;
	struct mux_config *mux;

	for(i = 0; i < config->mux_count; i++) {
		mux = &config->muxes[i];
		if (set_gpio_func_id(mux->func_id, mux->gpio->reg, mux->gpio->shift, regs[0])) {
			debug_print("invalid function id\n");
			return -1;
		}
		if (mux->gpio_dir >= 0)
			config->dev_info->set_gpio_dir(regs[1], mux->gpio->gpio, mux->gpio_dir);

		if (mux->gpio_val >= 0)
			config->dev_info->set_gpio_val(regs[1], mux->gpio->gpio, mux->gpio_val);
	}
	printf("Config loaded\n");

	return 0;
}

char *help = "Usage:"
    "	rtk_mux_ctl [OPTION]\n"
    "	-c <filename>	Config file (default: "DEFAULT_CONFIG_LOCATION")\n"
    "	-d		Only display current configuration\n"
    "	-s		Simulate - don't actually apply config\n"
    "	-o		Show available config options (signals)\n"
    "	-v		Verbose, print all detailed mux info\n";

int parse_args(int argc, char *argv[], int *opts, char *config_f)
{
	int opt;

	while ((opt = getopt(argc, argv, "dsvoc:")) != -1) {
		switch (opt) {
		case 'd':
			*opts |= MUX_DISPLAY_ONLY;
			break;
		case 'v':
			*opts |= MUX_VERBOSE;
			break;
		case 's':
			*opts |= MUX_SIMULATE;
			break;
		case 'c':
			strncpy(config_f, optarg, 255);
			break;
		case 'o':
			*opts |= MUX_SHOW_SIGNALS;
			break;
		default:
			printf("\n%s", help);
			return -1;
		}
	}

	return 0;
}

int assign_dev_info(struct config_info *config)
{
	FILE *fh;
	char line[256];

	fh = fopen("/tmp/sysinfo/board_name", "r");

	if (fh != NULL && line != NULL) {
		if (fread(line, 100, 1, fh) >= 0) {
			fclose(fh);
			if (strncmp(line, "kinkan", 6) == 0) {
				printf("Board: Kinkan\n");
				config->dev_info = &dev_kinkan;
			} else {
				printf("Unknown 8devices board\n");
				return -1;
			}
		}
		else {
			printf("Unknown 8devices board\n");
			return -1;
		}
	}
	else {
		printf("Unknown 8devices board\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct config_info config;
	struct device_in_out_func *info;

	uint32_t **regs;

	int opts = 0;
	char config_file[256] = DEFAULT_CONFIG_LOCATION;

	if (parse_args(argc, argv, &opts, config_file))
		return -1;

	if (assign_dev_info(&config))
		return -1;

	info = config.dev_info;

	if (opts & MUX_SHOW_SIGNALS) {
		if (!info->gpio_mux)
			return 1;

		printf("\nAvailable GPIO functions:\n");
		print_all_gpios(info);

		return 0;
	}

	regs = get_regs(info);
	if (regs == NULL) {
		printf("Failed to read registers\n");
		return -1;
	}
	if (opts & MUX_VERBOSE)
		dump_regs(regs, info);


	if (opts & (MUX_VERBOSE | MUX_DISPLAY_ONLY))
	{
		dump_current_config(regs, info);
	}

	if (opts & MUX_DISPLAY_ONLY)
		return 0;

	if (load_config(config_file, &config)) {
		printf("Failed to parse config, exiting\n");
		return -1;
	}

	if (check_function_overlay(config, info))
		return -1;

	if (opts & MUX_SIMULATE) {
		dump_config_changes(regs, &config);
		return 0;
	}

	apply_config_changes(&config, regs);
	set_regs(regs, info);

	if (opts & MUX_VERBOSE) {
		dump_regs(regs, info);
		dump_current_config(regs, info);
	}

	return 0;
}
