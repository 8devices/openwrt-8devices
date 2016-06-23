#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <jansson.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/mman.h>

#include "qca_data.h"

#define DIR_IN 1
#define DIR_OUT 0

#define DEFAULT_CONFIG_LOCATION "/etc/qca_mux_conf.json"

#define QCA955X_GPIO_BASE   0xb8040000
#define QCA955X_GPIO_BLOCK_SIZE   0x70
#define MMAP_PATH          "/dev/mem"

struct mux_config {
	int gpio;
	int gpio_direction;
	int signal_direction;
	char *signal;
};

struct function_config {
	int value;
	bool state;
	char name[100];
};

struct config_info {
	struct mux_config *muxes;
	int mux_count;
	struct function_config *functions;
	int function_count;
};



int mmap_setup(void **mmap_ptr)
{
	uint8_t *mmap_reg = NULL;
	int mmap_fd;

	if ((mmap_fd = open(MMAP_PATH, O_RDWR)) < 0) {
		printf("mmap: unable to open %s\n", MMAP_PATH);
		return -1;
	}

	mmap_reg = (uint8_t *) mmap(NULL, QCA955X_GPIO_BLOCK_SIZE, PROT_READ | PROT_WRITE,
				    MAP_FILE | MAP_SHARED, mmap_fd, QCA955X_GPIO_BASE);

	if (mmap_reg == MAP_FAILED) {
		close(mmap_fd);
		return -1;
	}
	close(mmap_fd);
	*mmap_ptr = (void *)mmap_reg;

	return 0;

}

void mmap_close(void *mmap_ptr)
{
	munmap(mmap_ptr, QCA955X_GPIO_BLOCK_SIZE);
}

void mmap_read_regs(int reg_size, void *mmap_ptr, uint32_t ** regs)
{
	int i;

	for (i = 0; i < reg_size; i += 4) {
		if (!(i == 0x0C || i == 0x10)) // Skip write only regs
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

uint32_t rmwl(uint32_t reg, uint32_t mask, uint32_t val)
{
	return ((reg & mask) | val);
}

uint32_t *get_regs_debug(void)
{
	int i;
	uint32_t *regs;

	regs = calloc(28, sizeof(uint32_t));
	if (!regs)
		return NULL;

	for (i = 0; i < 28; i++) {
		regs[i] = default_regs[i];
	}

	return regs;
}

uint32_t *get_regs(int reg_size)
{
	void *mmap_ptr;
	uint32_t *regs;

	regs = calloc(28, sizeof(uint32_t));
	if (!regs)
		return NULL;

	if (mmap_setup(&mmap_ptr))
		return NULL;

	mmap_read_regs(reg_size, mmap_ptr, &regs);
	mmap_close(mmap_ptr);

	return regs;
}

int set_regs(int reg_size, uint32_t * regs)
{
	void *mmap_ptr;

	if (mmap_setup(&mmap_ptr))
		return -1;

	mmap_write_regs(reg_size, mmap_ptr, regs);
	mmap_close(mmap_ptr);

	return 0;
}

void dump_regs(uint32_t * regs, struct device_in_out_func *info)
{
	int i;
	for (i = 0; i < info->reg_count; i++) {
		printf("%-18s 0x%02x: 0x%08x\n", info->reg_names[i], i * 4, regs[i]);
	}
}

int get_jtag_status(int gpio, uint32_t * regs)
{
	if (!(regs[0x6c / 4] & 0x2) && gpio >= 0 && gpio <= 3)
		return 1;
	else
		return 0;
}

uint32_t get_gpio_out_func(int gpio, uint32_t * regs)
{
	uint32_t word, byte, func;
//      if (!valid_gpio(gpio))
//              return ERR;

	word = gpio / 4 + 0x2c / 4;
	byte = gpio % 4;
	func = (regs[word] >> (byte * 8)) & 0xff;

	//printf("%x %x %x %x %x\n", gpio, word, byte, regs[word], func);

	return func;
}

uint32_t get_gpio_direction(int gpio, uint32_t * regs)
{
	return (regs[0] >> gpio) & 0x1;
}

void set_gpio_direction(int gpio, int direction, uint32_t * regs)
{
	direction &= 0x1;
	regs[0] = rmwl(regs[0], ~(1 << gpio), direction << gpio);
}

void set_gpio_out_func(int gpio, int func, uint32_t * regs)
{
	int word = gpio / 4 + 0x2c / 4;
	int shift = (gpio % 4) * 8;

	regs[word] = rmwl(regs[word], ~(0xff << shift), func << shift);

}

void set_gpio_func_state(int reg, int shift, int status, uint32_t * regs)
{
	int word = reg / 4;

	regs[word] = rmwl(regs[word], ~(1 << shift), status << shift);
}

int get_gpio_func_state(int reg, int shift, uint32_t * regs)
{

	int word = reg / 4;

	return (regs[word] >> shift) & 0x1;

}

void set_gpio_in_func(int gpio, struct gpio_input_reg *gpio_input_func, uint32_t * regs)
{
	int word, shift;
	word = gpio_input_func->reg / 4;
	shift = gpio_input_func->offset;

	gpio &= 0xff;
	regs[word] = rmwl(regs[word], ~(0xff << shift), gpio << shift);
}

struct gpio_input_reg *get_gpio_in_func(int gpio, uint32_t * regs, struct device_in_out_func *info)
{
	int i;
	int word, shift;

	for (i = 0; i <= info->input_signal_count; i++) {
		word = info->input_signals[i].reg / 4;
		shift = info->input_signals[i].offset;

		if (gpio == ((regs[word] >> shift) & 0xff)) {
			return &(info->input_signals[i]);
		}
	}

	return NULL;
}

void print_func(int gpio, uint32_t func, struct gpio_input_reg *in_func, struct device_in_out_func *info,
		uint32_t direction, uint32_t * regs)
{
	int i;
	const char *name;
	const char *dir;
	char buff[20];

	if (direction == 1) {	//IN
		name = "GPIO";
		dir = "INPUT";
		if (in_func)
			name = in_func->name;
	} else {
		name = NULL;
		dir = "OUTPUT";

		for (i = 0; i <= info->output_signal_count; i++) {
			if (info->output_signals[i].id == func) {
				name = info->output_signals[i].name;
				break;
			}
		}

		if (!name) {
			snprintf(buff, 20, "UNKNOWN(%d)", func);
			name = buff;
		}
	}

	printf("GPIO%-2d %-7s - %s\n", gpio, dir, name);
}

int config_set_gpio_signal(int gpio, int direction, char *signal, int signal_direction,
			   uint32_t * regs, struct device_in_out_func *info)
{
	int i;
	struct gpio_input_reg *oldfunc;
	int found = 0;

	if (direction > -1)
		set_gpio_direction(gpio, direction, regs);

	// When no signal supplied just set GPIO direction
	if (signal == NULL)
		return 0;

	//This part is used to give gpio a function
	if (signal_direction == DIR_IN && info->input_signals != NULL) {
		if (strncmp("GPIO", signal, strlen("GPIO")) == 0) {
			while ((oldfunc = get_gpio_in_func(gpio, regs, info)))
				set_gpio_in_func(0x80, oldfunc, regs);
			return 0;
		}

		for (i = 0; i < info->input_signal_count; i++) {
			if (strcmp(info->input_signals[i].name, signal) == 0) {
				found = 1;
				//Clean if GPIO was defined on old signal
				while ((oldfunc = get_gpio_in_func(gpio, regs, info)))
					set_gpio_in_func(0x80, oldfunc, regs);
				set_gpio_in_func(gpio, &(info->input_signals[i]), regs);
				break;
			}
		}

	} else if (signal_direction == DIR_OUT && info->output_signals != NULL) {
		for (i = 0; i < info->output_signal_count; i++) {
			if (strcmp(info->output_signals[i].name, signal) == 0) {
				found = 1;
				set_gpio_out_func(gpio, info->output_signals[i].id, regs);
				break;
			}
		}
	}

	if (found == 0)
		return -1;

	return 0;
}

int config_set_gpio_function(char *function, int state, uint32_t * regs,
			     struct device_in_out_func *info)
{
	int i;
	int found = 0;

	//This part is for configuring function status and if the status is true 
	//it sets the corresponding gpio with that function
	for (i = 0; i < info->gpio_function_count; i++) {
		if (info->gpio_functions != NULL && strcmp(info->gpio_functions[i].name, function) == 0) {
			found = 1;
			set_gpio_func_state(info->gpio_functions[i].reg,
					    info->gpio_functions[i].offset, state, regs);
		}
	}

	if (found == 0)
		return -1;

	return 0;
}

int get_gpio_function(int gpio, uint32_t * regs, char **func_name,  struct device_in_out_func *info)
{
	int i, n;
	uint32_t func_reg, func_bit;
	int func_state = 0;
	int func_flags;
	struct gpio_function group;

	for (n = 0; n < info->gpio_function_group_count; n++) {
		group = info->gpio_function_groups[n];
		if (gpio != group.gpio)
			continue;
		
		for (i = 0; i < info->gpio_function_count; i++) {
			if ((strcmp(group.en_func_name,
			     info->gpio_functions[i].name) == 0)) {

				func_reg = info->gpio_functions[i].reg;
				func_bit = info->gpio_functions[i].offset;
				func_flags = group.flags;
				func_state = get_gpio_func_state(func_reg, func_bit, regs);
				if (func_flags & FUNC_FLAG_INVERTED)
					func_state = !func_state;

				if (func_state) {
					strcpy(*func_name, group.name);
					return 0;
				}
			}
		}
	}
	return -1;
}

void parse_gpios(uint32_t * regs, struct device_in_out_func *info)
{
	int gpio;
	char* dir;
	uint32_t out_func;
	uint32_t direction;
	struct gpio_input_reg *in_func;

	char* func_name = (char *)malloc(100);
	for (gpio = 0; gpio < info->gpio_count; gpio++) {
		direction = get_gpio_direction(gpio, regs);
		if (direction == 1)
			dir = "INPUT";
		else
			dir = "OUTPUT";

		// check if any FUNCTION is enabled on GPIO
		// these have priority over muxes
		if (!(get_gpio_function(gpio, regs, &func_name, info))) {
			printf("GPIO%-2d %-7s - %s\n", gpio, dir, func_name);
		}
		// if chip has signal muxing, check them
		else if (info->input_signals && info->output_signals) {
			out_func = get_gpio_out_func(gpio, regs);
			in_func = get_gpio_in_func(gpio, regs, info);
			print_func(gpio, out_func, in_func, info, direction, regs);
		}
		// if nothing else found, pin is GPIO
		else {
			printf("GPIO%-2d %-7s - %s\n", gpio, dir, "GPIO");
		}
	}
	free(func_name);
}

int check_function_overlay(struct config_info config, struct device_in_out_func *info)
{//TODO: clean-up
	int i, n, pin_has_func, gpio;
	struct function_config *funcs = config.functions;
	for (gpio = 0; gpio < info->gpio_count; gpio++) {
		pin_has_func = 0;
		for (n = 0; n < info->gpio_function_group_count; n++) {
			for (i = 0; i < config.function_count; i++) {
				if (gpio == info->gpio_function_groups[n].gpio
				    && strcmp(info->gpio_function_groups[n].en_func_name, funcs[i].name) == 0) {
					if (funcs[i].state == 1) {
						if (pin_has_func == 1) {
							printf("Error GPIO = %d has multiple functions enabled \n",
							       gpio);
							return -1;
						}

						pin_has_func = 1;
					}
				}
			}
		}
	}
	return 1;
}

int load_config(char *config_file, struct config_info *config)
{
	int i;
	json_error_t error;
	json_t *root, *mux_list, *mux_el;
	json_t *js_gpio, *js_gpio_dir, *js_signal_dir, *js_signal;
	json_t *js_jtag, *function_list, *js_function_el;
	json_t *js_function_state;

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

		js_gpio = json_object_get(mux_el, "gpio");
		if (!json_is_integer(js_gpio)) {
			printf("json error: gpio is not ints\n");
			goto free_muxes;
		}
		(config->muxes)[i].gpio = json_integer_value(js_gpio);

		// GPIO direction (optional)
		js_gpio_dir = json_object_get(mux_el, "gpio_direction");
		if (js_gpio_dir) {
			if (!json_is_string(js_gpio_dir)) {
				printf("json error: GPIO direction is not string\n");
				goto free_muxes;
			}

			if (strncmp(json_string_value(js_gpio_dir), "IN", 2) == 0)
				(config->muxes)[i].gpio_direction = 1;
			else if (strncmp(json_string_value(js_gpio_dir), "OUT", 3) == 0)
				(config->muxes)[i].gpio_direction = 0;
			else {
				printf("json error: direction value invalid\n");
				goto free_muxes;
			}
		} else {
			(config->muxes)[i].gpio_direction = -1;
		}

		// signal direction (optional)
		js_signal_dir = json_object_get(mux_el, "signal_direction");
		if (js_signal_dir) {
			if (!json_is_string(js_signal_dir)) {
				printf("json error: signal direction is not string\n");
				goto free_muxes;
			}

			if (strncmp(json_string_value(js_signal_dir), "IN", 2) == 0)
				(config->muxes)[i].signal_direction = 1;

			else if (strncmp(json_string_value(js_signal_dir), "OUT", 3) == 0)
				(config->muxes)[i].signal_direction = 0;

			else {
				printf("json error: signal direction value invalid\n");
				goto free_muxes;
			}
		} else {
			(config->muxes)[i].signal_direction = -1;
		}

		//TODO validate signal
		js_signal = json_object_get(mux_el, "signal");
		if (json_is_string(js_signal)) {
			(config->muxes)[i].signal = calloc(1, 100);
			strncpy((config->muxes)[i].signal, json_string_value(js_signal), 99);
		}
		else if (!js_signal || json_is_null(js_signal)) {
			(config->muxes)[i].signal = NULL;
		}
		else {
			printf("json error: signal is not string or null\n");
			goto free_muxes;
		}
	}

	function_list = json_object_get(root, "functions");
	if (!json_is_array(function_list)) {
		printf("json error: function load error on line %d: %s\n", error.line, error.text);
		json_decref(root);
		return -1;
	}

	config->function_count = json_array_size(function_list);
	config->functions = calloc(config->function_count, sizeof(struct function_config));

	for (i = 0; i < config->function_count; i++) {
		js_function_el = json_array_get(function_list, i);
		//Function name
		js_jtag = json_object_get(js_function_el, "gpio_function");
		if (!json_is_string(js_jtag)) {
			printf("json error: GPIO_FUNCTION is not a string\n");
			goto free_functions;
		}
		strncpy((config->functions)[i].name, json_string_value(js_jtag), 99);

		//Function state
		js_function_state = json_object_get(js_function_el, "state");
		if (!json_is_boolean(js_function_state)) {
			printf("json error: function state is not boolean\n");
			goto free_functions;
		}

		if (json_is_true(js_function_state))
			(config->functions)[i].state = 1;

		else if (json_is_false(js_function_state))
			(config->functions)[i].state = 0;

		else {
			printf("json error: invalid function state value\n");
			goto free_functions;

		}

	}

	json_decref(root);

	return config->mux_count;
 free_muxes:
	json_decref(root);
	for (i=0; i<config->mux_count; i++)
		free((config->muxes)[i].signal);
	free(config->muxes);
	config->muxes = NULL;
	config->mux_count = 0;
	return -1;

 free_functions:
	json_decref(root);
	free(config->functions);
	config->functions = NULL;
	config->function_count = 0;
	return -1;

}

#define MUX_VERBOSE 0x0001
#define MUX_SIMULATE 0x0002
#define MUX_DISPLAY_ONLY 0x0004
#define MUX_PC_DEBUG 0x0008
#define MUX_SHOW_SIGNALS 0x0010

char *help = "Usage:"
    "	qca_mux_ctl [OPTION]\n"
    "	-c <filename>	Config file (default: "DEFAULT_CONFIG_LOCATION")\n"
    "	-d		Only display current configuration\n"
    "	-s		Simulate - don't actually apply config\n"
    "	-o		Show available config options (signals)\n"
    "	-v		Verbose, print all detailed mux info\n";

int main(int argc, char *argv[])
{
	int i;
	int opts = 0;
	int opt;
	int ret;
	uint32_t *regs;
	int config_cnt;
	char config_f[256] = DEFAULT_CONFIG_LOCATION;
	char *line = calloc(1, 100);
	FILE *fh;

	while ((opt = getopt(argc, argv, "dsvpoc:")) != -1) {
		switch (opt) {
		case 'd':
			opts |= MUX_DISPLAY_ONLY;
			break;
		case 'v':
			opts |= MUX_VERBOSE;
			break;
		case 's':
			opts |= MUX_SIMULATE;
			break;
		case 'c':
			strncpy(config_f, optarg, 255);
			break;
		case 'p':
			opts |= MUX_PC_DEBUG;
			break;
		case 'o':
			opts |= MUX_SHOW_SIGNALS;
			break;
		default:
			printf("\n%s", help);
			return -1;
		}
	}


	struct config_info config;
	config_cnt = load_config(config_f, &config);
	struct mux_config *muxes = config.muxes;
	struct function_config *funcs = config.functions;
	struct device_in_out_func *info;
	if (opts & MUX_PC_DEBUG) {
		fh = fopen("name", "r");
	} else {
		fh = fopen("/tmp/sysinfo/board_name", "r");
	}

	if (fh != NULL && line != NULL) {
		if (fread(line, 100, 1, fh) >= 0) {
			fclose(fh);
			if (strncmp(line, "rambutan", 8) == 0) {
				printf("Board: Rambutan\n");
				info = &dev_rambutan;
			} else if (strncmp(line, "lima", 4) == 0) {
				printf("Board: Lima\n");
				info = &dev_lima;
			} else if (strncmp(line, "carambola", 9) == 0) {
				printf("Board: Carambola\n");
				info = &dev_carambola;
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

	if (opts & MUX_SHOW_SIGNALS) {
		if(info->output_signals) {
			printf("\nAvailable output signals:\n");
			for (i=0; i < info->output_signal_count; i++)
				printf("%s\n", info->output_signals[i].name);
		}
		if(info->input_signals) {
			printf("\nAvailable input signals:\n");
			for (i=0; i < info->input_signal_count; i++)
				printf("%s\n", info->input_signals[i].name);
		}
		if (info->gpio_functions) {
			printf("\nAvailable functions:\n");
			for (i=0; i < info->gpio_function_count; i++)
				printf("%s\n", info->gpio_functions[i].name);
		}
		return 1;
	}

	if (config_cnt < 0) {
		printf("Failed to parse config.\n");
		return -1;
	}
	//Check if GPIO does not have multiple functions enabled
	if (info->gpio_function_groups != NULL && info->gpio_functions != NULL) {
		ret = check_function_overlay(config, info);
	}
	if (ret == -1)
		return -1;

	if (opts & MUX_PC_DEBUG)
		regs = get_regs_debug();
	else {
		regs = get_regs(info->reg_size);
		if (!regs) {
			printf("Failed to read registers\n");
			return -1;
		}
	}

	if (opts & MUX_VERBOSE)
		dump_regs(regs, info);
	if (opts & (MUX_VERBOSE | MUX_DISPLAY_ONLY))
		parse_gpios(regs, info);

	if (opts & MUX_DISPLAY_ONLY)
		return 0;

	//Set GPIO function
	for (i = 0; i < config.mux_count; i++) {
		printf("Config GPIO %d to %d as %s(%d)\n", muxes[i].gpio, muxes[i].gpio_direction,
		       muxes[i].signal, muxes[i].signal_direction);
		ret =
		    config_set_gpio_signal(muxes[i].gpio, muxes[i].gpio_direction, muxes[i].signal,
					   muxes[i].signal_direction, regs, info);
		if (ret) {
			printf("Unknown signal name: %s\n", muxes[i].signal);
			return -1;
		}
	}

	//Change FUNCTION states and set appropriate gpio with the enabled functions
	for (i = 0; i < config.function_count; i++) {
		printf("Configuring function %s to state %d\n", funcs[i].name, funcs[i].state);
		ret = config_set_gpio_function(funcs[i].name, funcs[i].state, regs, info);
		if (ret) {
			printf("Unknown %s function\n", funcs[i].name);
			return -1;
		}
	}

	if (opts & MUX_VERBOSE) {
		dump_regs(regs, info);
		parse_gpios(regs, info);
	}

	if (opts & MUX_SIMULATE)
		return 0;

	if (!(opts & MUX_PC_DEBUG))
		set_regs(info->reg_size, regs);

	return 0;
}
