/*
 * Copyright (c) 2014-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef _SHELL_IO_H
#define _SHELL_IO_H

#include "sw.h"
#include "sw_api.h"
#include "fal.h"
#define SW_TYPE_DEF(type, parser, show) {type, parser, show}
typedef struct
{
    sw_data_type_e data_type;
    sw_error_t(*param_check) ();
    void (*show_func) ();
} sw_data_type_t;

void  set_talk_mode(int mode);
int get_talk_mode(void);
void set_full_cmdstrp(char **cmdstrp);
int
get_jump(void);
sw_data_type_t * cmd_data_type_find(sw_data_type_e type);
void  cmd_strtol(char *str, a_uint32_t * arg_val);

sw_error_t __cmd_data_check_complex(char *info, char *defval, char *usage,
				sw_error_t(*chk_func)(), void *arg_val,
				a_uint32_t size);

sw_error_t cmd_data_check_portid(char *cmdstr, fal_port_t * val, a_uint32_t size);

sw_error_t cmd_data_check_portmap(char *cmdstr, fal_pbmp_t * val, a_uint32_t size);
sw_error_t cmd_data_check_confirm(char *cmdstr, a_bool_t def, a_bool_t * val, a_uint32_t size);

sw_error_t cmd_data_check_uint64(char *cmd_str, a_uint64_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_uint32(char *cmd_str, a_uint32_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_uint16(char *cmd_str, a_uint32_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_uint8(char *cmd_str, a_uint32_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_enable(char *cmd_str, a_uint32_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_pbmp(char *cmd_str, a_uint32_t * arg_val,
                               a_uint32_t size);
sw_error_t cmd_data_check_duplex(char *cmd_str, a_uint32_t * arg_val,
                                 a_uint32_t size);
sw_error_t cmd_data_check_speed(char *cmd_str, a_uint32_t * arg_val,
                                a_uint32_t size);
sw_error_t cmd_data_check_capable(char *cmd_str, a_uint32_t * arg_val,
                                  a_uint32_t size);
sw_error_t cmd_data_check_macaddr(char *cmdstr, void *val, a_uint32_t size);

void cmd_data_print_uint64(a_uint8_t * param_name, a_uint64_t * buf,
                           a_uint32_t size);
void cmd_data_print_uint32(a_char_t * param_name, a_uint32_t * buf,
                           a_uint32_t size);
void cmd_data_print_uint16(a_char_t * param_name, a_uint32_t * buf,
                           a_uint32_t size);
void cmd_data_print_uint8(a_uint8_t * param_name, a_uint32_t * buf,
                           a_uint32_t size);
void cmd_data_print_enable(a_char_t * param_name, a_uint32_t * buf,
                           a_uint32_t size);
void cmd_data_print_pbmp(a_uint8_t * param_name, a_uint32_t * buf,
                         a_uint32_t size);
void cmd_data_print_duplex(a_uint8_t * param_name, a_uint32_t * buf,
                           a_uint32_t size);
void cmd_data_print_speed(a_uint8_t * param_name, a_uint32_t * buf,
                          a_uint32_t size);
void cmd_data_print_capable(a_uint8_t * param_name, a_uint32_t * buf,
                            a_uint32_t size);
void cmd_data_print_macaddr(a_char_t * param_name, a_uint32_t * buf,
                            a_uint32_t size);
void
cmd_data_print_cable_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);
void
cmd_data_print_cable_len(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);
void
cmd_data_print_ssdk_cfg(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);
sw_error_t
cmd_data_check_crossover_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size);

sw_error_t
cmd_data_check_crossover_status(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size);
sw_error_t
cmd_data_check_interface_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size);
void
cmd_data_print_crossover_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_crossover_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_interface_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_counter_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_phy_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);

void
cmd_data_print_debug_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size);
#endif
