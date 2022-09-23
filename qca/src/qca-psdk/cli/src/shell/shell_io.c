/*
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
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


#include <stdio.h>
#include "shell_io.h"
#include "shell.h"

#define SW_RTN_ON_NULL_PARAM(rtn) \
    do { if ((rtn) == NULL) return SW_BAD_PARAM; } while(0);

#define DEFAULT_FLAG "default"
static char **full_cmdstrp;
static int talk_mode = 1;

char g_aclcmd[500] = "\0";
a_uint32_t g_aclcmd_len = 0;;

void append_acl_cmd(char * cmd)
{
	if(500 > (g_aclcmd_len+1)) {
		g_aclcmd_len += snprintf(g_aclcmd+g_aclcmd_len, 500-g_aclcmd_len, "%s", cmd);
		g_aclcmd_len += snprintf(g_aclcmd+g_aclcmd_len, 500-g_aclcmd_len, "%s", " ");
	}
}

int
get_talk_mode(void)
{
    return talk_mode ;
}

void
set_talk_mode(int mode)
{
    talk_mode = mode;
}

char ** full_cmdstrp_bak;

void
set_full_cmdstrp(char **cmdstrp)
{
    full_cmdstrp = cmdstrp;
    full_cmdstrp_bak = cmdstrp;
}

int
get_jump(void)
{
    return (full_cmdstrp-full_cmdstrp_bak);
}

static char *
get_cmd_buf(char *tag, char *defval)
{
    if(!full_cmdstrp || !(*full_cmdstrp))
    {
        dprintf("parameter (%s) or default (%s) absent\n", tag, defval);
        exit(1);
    }

    if (!strcasecmp(*(full_cmdstrp), DEFAULT_FLAG))
    {
        full_cmdstrp++;
        return defval;
    }
    else
    {
        return *(full_cmdstrp++);
    }
}

static char *
get_cmd_stdin(char *tag, char *defval)
{
    static char gsubcmdstr[256];
    int pos = 0;
    int c;

    if(defval)
    {
        dprintf("%s(%s): ", tag, defval);
    }
    else
    {
        dprintf("%s: ", tag);
    }

    fflush(stdout);
    memset(gsubcmdstr, 0, sizeof(gsubcmdstr));

    while ((c = getchar()) != '\n')
    {
        gsubcmdstr[pos++] = c;
        if (pos == (sizeof(gsubcmdstr) - 1))
        {
            dprintf("too long command\n");
            return NULL;
        }
    }

    gsubcmdstr[pos] = '\0';
    if ('\0' == gsubcmdstr[0])
    {
        return defval;
    }
    else
    {
        return gsubcmdstr;
    }
}

static char *
get_sub_cmd(char *tag, char *defval)
{
    if(talk_mode)
        return get_cmd_stdin(tag, defval);
    else
        return get_cmd_buf(tag, defval);
}


static inline  a_bool_t
is_hex(char c)
{
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F'))
        return A_TRUE;

    return A_FALSE;
}

static inline a_bool_t
is_dec(char c)
{
    if ((c >= '0') && (c <= '9'))
        return A_TRUE;

    return A_FALSE;
}

static sw_data_type_t sw_data_type[] =
{
    SW_TYPE_DEF(SW_UINT8, cmd_data_check_uint8, cmd_data_print_uint8),
    SW_TYPE_DEF(SW_INT8, NULL, NULL),
    SW_TYPE_DEF(SW_UINT16, cmd_data_check_uint16, cmd_data_print_uint16),
    SW_TYPE_DEF(SW_INT16, NULL, NULL),
    SW_TYPE_DEF(SW_UINT32, cmd_data_check_uint32, cmd_data_print_uint32),
    SW_TYPE_DEF(SW_INT32, NULL, NULL),
    SW_TYPE_DEF(SW_UINT64, cmd_data_check_uint64, cmd_data_print_uint64),
    SW_TYPE_DEF(SW_INT64, NULL, NULL),
    SW_TYPE_DEF(SW_CAP, cmd_data_check_capable, cmd_data_print_capable),
    SW_TYPE_DEF(SW_DUPLEX, cmd_data_check_duplex, cmd_data_print_duplex),
    SW_TYPE_DEF(SW_SPEED, cmd_data_check_speed, cmd_data_print_speed),
    SW_TYPE_DEF(SW_PBMP, cmd_data_check_pbmp, cmd_data_print_pbmp),
    SW_TYPE_DEF(SW_ENABLE, cmd_data_check_enable, cmd_data_print_enable),
    SW_TYPE_DEF(SW_MACADDR, cmd_data_check_macaddr, cmd_data_print_macaddr),
    SW_TYPE_DEF(SW_CABLESTATUS, NULL, cmd_data_print_cable_status),
    SW_TYPE_DEF(SW_CABLELEN, NULL, cmd_data_print_cable_len),
    SW_TYPE_DEF(SW_SSDK_CFG, NULL, cmd_data_print_ssdk_cfg),
	SW_TYPE_DEF(SW_CROSSOVER_MODE, cmd_data_check_crossover_mode, cmd_data_print_crossover_mode),
    SW_TYPE_DEF(SW_CROSSOVER_STATUS, cmd_data_check_crossover_status, cmd_data_print_crossover_status),
    SW_TYPE_DEF(SW_INTERFACE_MODE, cmd_data_check_interface_mode, cmd_data_print_interface_mode),
    SW_TYPE_DEF(SW_COUNTER_INFO, NULL, cmd_data_print_counter_info),
    SW_TYPE_DEF(SW_REG_DUMP, NULL, cmd_data_print_register_info),
    SW_TYPE_DEF(SW_PHY_DUMP, NULL, cmd_data_print_phy_register_info),
    SW_TYPE_DEF(SW_DBG_REG_DUMP, NULL, cmd_data_print_debug_register_info),
};

sw_data_type_t *
cmd_data_type_find(sw_data_type_e type)
{
    a_uint16_t i = 0;

    do
    {
        if (type == sw_data_type[i].data_type)
            return &sw_data_type[i];
    }
    while ( ++i < sizeof(sw_data_type)/sizeof(sw_data_type[0]));

    return NULL;
}

sw_error_t __cmd_data_check_quit_help(char *cmd, char *usage)
{
    sw_error_t ret = SW_OK;

    if (!strncasecmp(cmd, "quit", 4)) {
        return SW_ABORTED;
    } else if (!strncasecmp(cmd, "help", 4)) {
        dprintf("%s", usage);
        ret = SW_BAD_VALUE;
    }

    return ret;
}

sw_error_t __cmd_data_check_complex(char *info, char *defval, char *usage,
				sw_error_t(*chk_func)(), void *arg_val,
				a_uint32_t size)
{
    sw_error_t ret;
    char *cmd;

    do {
        cmd = get_sub_cmd(info, defval);
        SW_RTN_ON_NULL_PARAM(cmd);

        ret = __cmd_data_check_quit_help(cmd, usage);
        if (ret == SW_ABORTED)
            return ret;
        else if (ret == SW_OK) {
            ret = chk_func(cmd, arg_val, size);
            if (ret)
                dprintf("%s", usage);
        }
    } while (talk_mode && (SW_OK != ret));

    return SW_OK;
}

sw_error_t
cmd_data_check_uint8(char *cmd_str, a_uint32_t *arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    if (255 < *arg_val)
    {
        return SW_BAD_PARAM;
    }

    return SW_OK;
}

void
cmd_data_print_uint8(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint8_t *) buf);

}


sw_error_t
cmd_data_check_uint32(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (strspn(cmd_str, "1234567890abcdefABCDEFXx") != strlen(cmd_str)){
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    return SW_OK;
}

void
cmd_data_print_uint32(a_char_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint32_t *) buf);
}

sw_error_t
cmd_data_check_uint64(char *cmd_str, a_uint64_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (strspn(cmd_str, "1234567890abcdefABCDEFXx") != strlen(cmd_str)){
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%llx", arg_val);
    else
        sscanf(cmd_str, "%lld", arg_val);

    return SW_OK;
}

void
cmd_data_print_uint64(a_uint8_t * param_name, a_uint64_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%llx", param_name, *(a_uint64_t *) buf);
}

sw_error_t
cmd_data_check_uint16(char *cmd_str, a_uint32_t *arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (0 == cmd_str[0])
    {
        return SW_BAD_VALUE;
    }

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    if (65535 < *arg_val)
    {
        return SW_BAD_PARAM;
    }

    return SW_OK;
}

void
cmd_data_print_uint16(a_char_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%04x", param_name, *(a_uint16_t *) buf);

}

sw_error_t
cmd_data_check_pbmp(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (cmd_str[0] == '0' && (cmd_str[1] == 'x' || cmd_str[1] == 'X'))
        sscanf(cmd_str, "%x", arg_val);
    else
        sscanf(cmd_str, "%d", arg_val);

    return SW_OK;

}

void
cmd_data_print_pbmp(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:0x%x", param_name, *(a_uint32_t *) buf);

}

sw_error_t
cmd_data_check_enable(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "disable"))
        *arg_val = FAL_DISABLE;
    else if (!strcasecmp(cmd_str, "enable"))
        *arg_val = FAL_ENABLE;
    else
    {
        //dprintf("input error");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_enable(a_char_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == 1)
    {
        dprintf("ENABLE");
    }
    else if (*(a_uint32_t *) buf == 0)
    {
        dprintf("DISABLE");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}
/*port counter*/
static char *counter_regname[] =
{
    "RxGoodFrame",
    "RxBadCRC   ",
    "TxGoodFrame",
    "TxBadCRC   ",
    "SysRxGoodFrame",
    "SysRxBadCRC",
    "SysTxGoodFrame",
    "SysTxBadCRC",
};

void
cmd_data_print_counter_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s] \n", param_name);
    a_uint32_t offset = 0;
    for (offset = 0; offset < (sizeof (fal_port_counter_info_t) / sizeof (a_uint32_t));
            offset++)
    {

        dprintf("%s<0x%08x>\n", counter_regname[offset], *(buf + offset));

    }
}

void
cmd_data_print_debug_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s]", param_name);
	fal_debug_reg_dump_t * reg_dump = (fal_debug_reg_dump_t * )buf;

	a_uint32_t reg_count;

	dprintf("\n%s. ", reg_dump->reg_name);

	reg_count = 0;
	dprintf("\n");
	for (;reg_count < reg_dump->reg_count;reg_count++)
	{
		dprintf("%08x:%08x  ",reg_dump->reg_addr[reg_count], reg_dump->reg_value[reg_count]);
		if ((reg_count + 1) % 4 == 0)
			dprintf("\n");
	}

	dprintf("\n\n\n");
}



void
cmd_data_print_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s]", param_name);
	fal_reg_dump_t * reg_dump = (fal_reg_dump_t * )buf;
	a_uint32_t n[8]={0,4,8,0xc,0x10,0x14,0x18,0x1c};
	a_uint32_t dump_addr, reg_count;

	dprintf("\n%s. ", reg_dump->reg_name);
	dprintf("\n	%8x %8x %8x %8x %8x %8x %8x %8x\n",
					n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
	dprintf(" [%04x] ", reg_dump->reg_base);

	reg_count = 0;
	for (dump_addr = reg_dump->reg_base;
			(dump_addr <= reg_dump->reg_end )&& (reg_count <= reg_dump->reg_count);
			reg_count++)
	{
		dprintf("%08x ", reg_dump->reg_value[reg_count]);
		dump_addr += 4;
		if ((reg_count + 1) % 8 == 0)
			dprintf("\n [%04x] ", dump_addr);
	}

	dprintf("\n\n\n");
}

void
cmd_data_print_phy_register_info(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("\n[%s]", param_name);
	fal_phy_dump_t * phy_dump = (fal_phy_dump_t * )buf;

	a_uint32_t n[8]={0,1,2,3,4,5,6,7};

	a_uint32_t dump_addr, reg_count;

	dprintf("\n%s. ", phy_dump->phy_name);
	dprintf("\n	%8x %8x %8x %8x %8x %8x %8x %8x\n",
					n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
	dprintf(" [%04x] ", phy_dump->phy_base);

	reg_count = 0;
	for (dump_addr = phy_dump->phy_base;
			(dump_addr <= phy_dump->phy_end )&& (reg_count <= phy_dump->phy_count);
			reg_count++)
	{
		dprintf("%08x ", phy_dump->phy_value[reg_count]);
		dump_addr ++;
		if ((reg_count + 1) % 8 == 0)
			dprintf("\n [%04x] ", dump_addr);
	}

	dprintf("\n\n\n");
}


/*port ctrl*/
sw_error_t
cmd_data_check_duplex(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strcasecmp(cmd_str, "half"))
        *arg_val = FAL_HALF_DUPLEX;
    else if (!strcasecmp(cmd_str, "full"))
        *arg_val = FAL_FULL_DUPLEX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_duplex(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == 0)
    {
        dprintf("HALF");
    }
    else if (*(a_uint32_t *) buf == 1)
    {
        dprintf("FULL");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_speed(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "10", 3))
        *arg_val = FAL_SPEED_10;
    else if (!strncasecmp(cmd_str, "100", 4))
        *arg_val = FAL_SPEED_100;
    else if (!strncasecmp(cmd_str, "1000", 5))
        *arg_val = FAL_SPEED_1000;
    else if (!strncasecmp(cmd_str, "2500", 5))
        *arg_val = FAL_SPEED_2500;
    else if (!strncasecmp(cmd_str, "5000", 5))
        *arg_val = FAL_SPEED_5000;
    else if (!strncasecmp(cmd_str, "10000", 6))
        *arg_val = FAL_SPEED_10000;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_speed(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == FAL_SPEED_10)
    {
        dprintf("10(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_100)
    {
        dprintf("100(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_1000)
    {
        dprintf("1000(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_2500)
    {
        dprintf("2500(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_5000)
    {
        dprintf("5000(Mbps)");
    }
    else if (*(a_uint32_t *) buf == FAL_SPEED_10000)
    {
        dprintf("10000(Mbps)");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_capable(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    cmd_strtol(cmd_str, arg_val);
    if (*arg_val & (~FAL_PHY_COMBO_ADV_ALL))
    {
        //dprintf("input error should be within 0x3f\n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_capable(a_uint8_t * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == 0)
    {
        dprintf("None Capable");
        return;
    }

    if (*(a_uint32_t *) buf & FAL_PHY_ADV_10000T_FD)
    {
        dprintf("10000TX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_5000T_FD)
    {
        dprintf("5000TX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_2500T_FD)
    {
        dprintf("2500TX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000BX_FD)
    {
        dprintf("1000BX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000BX_HD)
    {
        dprintf("1000BX_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_1000T_FD)
    {
        dprintf("1000T_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_100TX_FD)
    {
        dprintf("100TX_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_100TX_HD)
    {
        dprintf("100TX_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_10T_HD)
    {
        dprintf("10T_HD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_10T_FD)
    {
        dprintf("10T_FD|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_PAUSE)
    {
        dprintf("PAUSE|");
    }
    if (*(a_uint32_t *) buf & FAL_PHY_ADV_ASY_PAUSE)
    {
        dprintf("ASY_PAUSE|");
    }
}

sw_error_t
cmd_data_check_crossover_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "auto", 5))
        *arg_val = PHY_MDIX_AUTO;
    else if (!strncasecmp(cmd_str, "mdi", 4))
        *arg_val = PHY_MDIX_MDI;
    else if (!strncasecmp(cmd_str, "mdix", 5))
        *arg_val = PHY_MDIX_MDIX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_crossover_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_MDIX_AUTO)
    {
        dprintf("AUTO");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_MDI)
    {
        dprintf("MDI");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_MDIX)
    {
        dprintf("MDIX");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

sw_error_t
cmd_data_check_crossover_status(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;
    if (!strncasecmp(cmd_str, "mdi", 4))
        *arg_val = PHY_MDIX_STATUS_MDI;
    else if (!strncasecmp(cmd_str, "mdix", 5))
        *arg_val = PHY_MDIX_STATUS_MDIX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_crossover_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_MDIX_STATUS_MDI)
    {
        dprintf("MDI");
    }
    else if (*(a_uint32_t *) buf == PHY_MDIX_STATUS_MDIX)
    {
        dprintf("MDIX");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}
sw_error_t
cmd_data_check_interface_mode(char *cmd_str, a_uint32_t * arg_val, a_uint32_t size)
{
    if (cmd_str == NULL)
        return SW_BAD_PARAM;

    if (!strncasecmp(cmd_str, "psgmii_baset", 13))
        *arg_val = PHY_PSGMII_BASET;
    else if (!strncasecmp(cmd_str, "psgmii_bx1000", 14))
        *arg_val = PHY_PSGMII_BX1000;
    else if (!strncasecmp(cmd_str, "psgmii_fx100", 13))
        *arg_val = PHY_PSGMII_FX100;
    else if (!strncasecmp(cmd_str, "psgmii_amdet", 13))
        *arg_val = PHY_PSGMII_AMDET;
    else if (!strncasecmp(cmd_str, "rgmii_amdet", 13))
        *arg_val = PORT_RGMII_AMDET;
    else if (!strncasecmp(cmd_str, "rgmii_baset", 13))
        *arg_val = PORT_RGMII_BASET;
    else if (!strncasecmp(cmd_str, "rgmii_bx1000", 13))
        *arg_val = PORT_RGMII_BX1000;
    else if (!strncasecmp(cmd_str, "rgmii_fx100", 13))
        *arg_val = PORT_RGMII_FX100;
    else if (!strncasecmp(cmd_str, "sgmii_baset", 13))
        *arg_val = PHY_SGMII_BASET;
    else if (!strncasecmp(cmd_str, "qsgmii", 13))
	  *arg_val = PORT_QSGMII;
    else if (!strncasecmp(cmd_str, "sgmii_plus", 13))
	  *arg_val = PORT_SGMII_PLUS;
    else if (!strncasecmp(cmd_str, "usxgmii", 13))
	  *arg_val = PORT_USXGMII;
    else if (!strncasecmp(cmd_str, "10gbase_r", 13))
	  *arg_val = PORT_10GBASE_R;
    else if (!strncasecmp(cmd_str, "sgmii_fiber", 20))
	  *arg_val = PORT_SGMII_FIBER;
    else if (!strncasecmp(cmd_str, "psgmii_fiber", 20))
	  *arg_val = PHY_PSGMII_FIBER;
    else if (!strncasecmp(cmd_str, "interfacemode_max", 20))
	  *arg_val = PORT_INTERFACE_MODE_MAX;
    else
    {
        //dprintf("input error \n");
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_interface_mode(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);
    if (*(a_uint32_t *) buf == PHY_PSGMII_BASET)
    {
        dprintf("PSGMII_BASET");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_BX1000)
    {
        dprintf("PSGMII_BX1000");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_FX100)
    {
	    dprintf("PSGMII_FX100");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_AMDET)
    {
	    dprintf("PSGMII_AMDET");
    }
    else if (*(a_uint32_t *) buf == PORT_RGMII_AMDET)
    {
	    dprintf("RGMII_AMDET");
    }
    else if (*(a_uint32_t *) buf == PORT_RGMII_BASET)
    {
	    dprintf("RGMII_BASET");
    }
    else if (*(a_uint32_t *) buf == PORT_RGMII_BX1000)
    {
	    dprintf("RGMII_BX1000");
    }
    else if (*(a_uint32_t *) buf == PORT_RGMII_FX100)
    {
	    dprintf("RGMII_FX100");
    }
    else if (*(a_uint32_t *) buf == PHY_SGMII_BASET)
    {
	    dprintf("SGMII_BASET");
    }
    else if (*(a_uint32_t *) buf == PORT_QSGMII)
    {
	    dprintf("QSGMII");
    }
    else if (*(a_uint32_t *) buf == PORT_SGMII_PLUS)
    {
	    dprintf("SGMII PLUS");
    }
    else if (*(a_uint32_t *) buf == PORT_USXGMII)
    {
	    dprintf("USXGMII");
    }
    else if (*(a_uint32_t *) buf == PORT_10GBASE_R)
    {
	    dprintf("10gbase_r");
    }
    else if (*(a_uint32_t *) buf == PORT_SGMII_FIBER)
    {
	    dprintf("sgmii_fiber");
    }
    else if (*(a_uint32_t *) buf == PHY_PSGMII_FIBER)
    {
	    dprintf("psgmii_fiber");
    }
    else if (*(a_uint32_t *) buf == PORT_INTERFACE_MODE_MAX)
    {
	    dprintf("INTERFACEMODE_MAX");
    }
    else
    {
	    dprintf("UNKNOWN VALUE");
    }
}
sw_error_t
cmd_data_check_confirm(char *cmdstr, a_bool_t def, a_bool_t * val,
                       a_uint32_t size)
{
    if (0 == cmdstr[0])
    {
        *val = def;
    }
    else if ((!strcasecmp(cmdstr, "yes")) || (!strcasecmp(cmdstr, "y")))
    {
        *val = A_TRUE;
    }
    else if ((!strcasecmp(cmdstr, "no")) || (!strcasecmp(cmdstr, "n")))
    {
        *val = A_FALSE;
    }
    else
    {
        return SW_BAD_VALUE;
    }

    return SW_OK;
}

void
cmd_data_print_confirm(char * param_name, a_bool_t val, a_uint32_t size)
{
    dprintf("%s", param_name);
    if (A_TRUE == val)
    {
        dprintf("YES");
    }
    else
    {
        dprintf("NO");
    }

    return;
}

sw_error_t
cmd_data_check_portid(char *cmdstr, fal_port_t * val, a_uint32_t size)
{
    *val = 0;
    //default input null
    if(!strcasecmp(cmdstr, "null"))
    {
        if (ssdk_cfg.init_cfg.chip_type == CHIP_HPPE)
            return SW_BAD_VALUE;
        return SW_OK;
    }
   if (strstr(cmdstr, "0x") == NULL)
	sscanf(cmdstr, "%d", val);
   else
	sscanf(cmdstr, "%x", val);

    return SW_OK;
}

sw_error_t
cmd_data_check_portmap(char *cmdstr, fal_pbmp_t * val, a_uint32_t size)
{
    char *tmp = NULL, *str_save;
    a_uint32_t port;

    *val = 0;
    //default input null
    if(!strcasecmp(cmdstr, "null"))
    {
        return SW_OK;
    }

    tmp = (void *) strtok_r(cmdstr, ",", &str_save);
    while (tmp)
    {
        sscanf(tmp, "%d", &port);
        if (SW_MAX_NR_PORT <= port)
        {
            return SW_BAD_VALUE;
        }

        *val |= (0x1 << port);
        tmp = (void *) strtok_r(NULL, ",", &str_save);
    }

    return SW_OK;
}

void
cmd_data_print_portmap(char * param_name, fal_pbmp_t val, a_uint32_t size)
{
    a_uint32_t i;
    char tmp[16];
    tmp[0] = '\0';

    dprintf("%s", param_name);
    for (i = 0; i < SW_MAX_NR_PORT; i++)
    {
        if (val & (0x1 << i))
        {
            if(strlen(tmp) == 0)
                snprintf(tmp, sizeof(tmp), "%d", i);
            else
                snprintf(tmp+strlen(tmp), sizeof(tmp+strlen(tmp)), ",%d", i);
        }
    }
    dprintf("%s ", tmp);
    return;
}

sw_error_t
cmd_data_check_macaddr(char *cmdstr, void *val, a_uint32_t size)
{
    char *tmp = NULL, *str_save;
    a_uint32_t i = 0, j;
    a_uint32_t addr;
    fal_mac_addr_t mac;

    memset(&mac, 0, sizeof (fal_mac_addr_t));
    if (NULL == cmdstr)
    {
        *(fal_mac_addr_t *) val = mac;
        return SW_BAD_VALUE; /*was: SW_OK;*/
    }

    if (0 == cmdstr[0])
    {
        *(fal_mac_addr_t *) val = mac;
        return SW_OK;
    }

    tmp = (void *) strtok_r(cmdstr, "-", &str_save);
    while (tmp)
    {
        if (6 <= i)
        {
            return SW_BAD_VALUE;
        }

        if ((2 < strlen(tmp)) || (0 == strlen(tmp)))
        {
            return SW_BAD_VALUE;
        }

        for (j = 0; j < strlen(tmp); j++)
        {
            if (A_FALSE == is_hex(tmp[j]))
                return SW_BAD_VALUE;
        }

        sscanf(tmp, "%x", &addr);
        if (0xff < addr)
        {
            return SW_BAD_VALUE;
        }

        mac.uc[i++] = addr;
        tmp = (void *) strtok_r(NULL, "-", &str_save);
    }

    if (6 != i)
    {
        return SW_BAD_VALUE;
    }

    *(fal_mac_addr_t *) val = mac;
    return SW_OK;
}

void
cmd_data_print_macaddr(a_char_t * param_name, a_uint32_t * buf,
                       a_uint32_t size)
{
    a_uint32_t i;
    fal_mac_addr_t *val;

    val = (fal_mac_addr_t *) buf;
    dprintf("%s", param_name);
    for (i = 0; i < 5; i++)
    {
        dprintf("%02x-", val->uc[i]);
    }
    dprintf("%02x", val->uc[5]);
    fflush(stdout);

}

#define cmd_data_check_element(info, defval, usage, chk_func, param) \
{\
    sw_error_t ret;\
    do {\
        cmd = get_sub_cmd(info, defval);\
        SW_RTN_ON_NULL_PARAM(cmd);\
        \
        if (!strncasecmp(cmd, "quit", 4)) {\
            return SW_BAD_VALUE;\
        } else if (!strncasecmp(cmd, "help", 4)) {\
            dprintf("%s", usage);\
            ret = SW_BAD_VALUE;\
        } else {\
            ret = chk_func param; \
            if (SW_OK != ret)\
                dprintf("%s", usage);\
            else\
            append_acl_cmd(cmd);\
        }\
    } while (talk_mode && (SW_OK != ret));\
}

sw_error_t
cmd_data_check_integer(char *cmd_str, a_uint32_t * arg_val, a_uint32_t max_val,
                       a_uint32_t min_val)
{
    a_uint32_t tmp;
    a_uint32_t i;

    if (NULL == cmd_str)
    {
        return SW_BAD_PARAM;
    }

    if (0 == cmd_str[0])
    {
        return SW_BAD_PARAM;
    }

    if ((cmd_str[0] == '0') && ((cmd_str[1] == 'x') || (cmd_str[1] == 'X')))
    {
        for (i = 2; i < strlen(cmd_str); i++)
        {
            if (A_FALSE == is_hex(cmd_str[i]))
            {
                return SW_BAD_VALUE;
            }
        }
        sscanf(cmd_str, "%x", &tmp);
    }
    else
    {
        for (i = 0; i < strlen(cmd_str); i++)
        {
            if (A_FALSE == is_dec(cmd_str[i]))
            {
                return SW_BAD_VALUE;
            }
        }
        sscanf(cmd_str, "%d", &tmp);
    }

    if ((tmp > max_val) || (tmp < min_val))
        return SW_BAD_PARAM;

    *arg_val = tmp;
    return SW_OK;
}
void
cmd_data_print_cable_status(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:", param_name);

    if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_NORMAL)
    {
        dprintf("NORMAL");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_SHORT)
    {
        dprintf("SHORT");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_OPENED)
    {
        dprintf("OPENED");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_INVALID)
    {
        dprintf("INVALID");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_CROSSOVERA)
    {
        dprintf("CROSSOVERA");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_CROSSOVERB)
    {
        dprintf("CROSSOVERB");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_CROSSOVERC)
    {
        dprintf("CROSSOVERC");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_CROSSOVERD)
    {
        dprintf("CROSSOVERD");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_LOW_MISMATCH)
    {
        dprintf("LOW_MISMATCH");
    }
    else if (*(a_uint32_t *) buf == FAL_CABLE_STATUS_HIGH_MISMATCH)
    {
        dprintf("HIGH_MISMATCH");
    }
    else
    {
        dprintf("UNKNOWN VALUE");
    }
}

void
cmd_data_print_cable_len(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    dprintf("[%s]:%d", param_name, *(a_uint32_t *) buf);
}

char*
cmd_cpu_mode(hsl_init_mode mode)
{
    switch (mode)
    {
        case HSL_NO_CPU:
            return "no_cpu";
        case HSL_CPU_1:
            return "cpu_1";
        case HSL_CPU_2:
            return "cpu_2";
        case HSL_CPU_1_PLUS:
            return "cpu_1_plus";
    }

    return "unknow";
}

char*
cmd_access_mode(hsl_access_mode mode)
{
    switch (mode)
    {
        case HSL_MDIO:
            return "mdio";
        case HSL_HEADER:
            return "header";
    }

    return "unknow";
}

static void
_cmd_collect_shell_cfg(ssdk_cfg_t *shell_cfg)
{
    memset(shell_cfg, 0, sizeof(ssdk_cfg_t));
    shell_cfg->init_cfg = init_cfg;

#ifdef VERSION
    aos_mem_copy(shell_cfg->build_ver, VERSION, sizeof(VERSION));
#endif

#ifdef BUILD_DATE
    aos_mem_copy(shell_cfg->build_date, BUILD_DATE, sizeof(BUILD_DATE));
#endif

    if (ssdk_cfg.init_cfg.chip_type == CHIP_ATHENA)
        aos_mem_copy(shell_cfg->chip_type, "athena", sizeof("athena"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_GARUDA)
        aos_mem_copy(shell_cfg->chip_type, "garuda", sizeof("garuda"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_SHIVA)
        aos_mem_copy(shell_cfg->chip_type, "shiva", sizeof("shiva"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_HORUS)
        aos_mem_copy(shell_cfg->chip_type, "horus", sizeof("horus"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_ISIS)
        aos_mem_copy(shell_cfg->chip_type, "isis", sizeof("isis"));
    else if (ssdk_cfg.init_cfg.chip_type == CHIP_ISISC)
        aos_mem_copy(shell_cfg->chip_type, "isisc", sizeof("isisc"));

#ifdef CPU
    aos_mem_copy(shell_cfg->cpu_type, CPU, sizeof(CPU));
#endif

#ifdef OS
    aos_mem_copy(shell_cfg->os_info, OS, sizeof(OS));
#if defined KVER26
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version 2.6", sizeof(" version 2.6"));
#elif defined KVER24
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version 2.4", sizeof(" version 2.4"));
#else
    aos_mem_copy(shell_cfg->os_info+sizeof(OS)-1, " version unknown", sizeof(" version unknown"));
#endif
#endif

#ifdef HSL_STANDALONG
    shell_cfg->fal_mod = A_FALSE;
#else
    shell_cfg->fal_mod = A_TRUE;
#endif

#ifdef USER_MODE
    shell_cfg->kernel_mode = A_FALSE;
#else
    shell_cfg->kernel_mode = A_TRUE;
#endif

#ifdef UK_IF
    shell_cfg->uk_if = A_TRUE;
#else
    shell_cfg->uk_if = A_FALSE;
#endif

    return;
}

#define BOOL2STR(val_bool) (((val_bool)==A_TRUE)?"true":"false" )
static void
_cmd_data_print_cfg(ssdk_cfg_t *entry)
{
    ssdk_init_cfg *init = &(entry->init_cfg);

    dprintf("[build verison]:%-10s [build date]:%s\n", entry->build_ver, entry->build_date);
    dprintf("[chip type]:%-14s [arch]:%-12s [os]:%s\n", entry->chip_type, entry->cpu_type, entry->os_info);
    dprintf("[fal]:%-20s [kernel mode]:%-5s [uk if]:%s\n",
            BOOL2STR(entry->fal_mod), BOOL2STR(entry->kernel_mode), BOOL2STR(entry->uk_if));

    dprintf("[cpu mode]:%-15s [reg access]:%-6s [ioctl minor]:%d\n",
            cmd_cpu_mode(init->cpu_mode), cmd_access_mode(init->reg_mode),
            init->nl_prot);
}

void
cmd_data_print_ssdk_cfg(a_uint8_t * param_name, a_uint32_t * buf, a_uint32_t size)
{
    ssdk_cfg_t *ssdk_cfg = (ssdk_cfg_t *) buf;
    dprintf("1.SSDK CONFIGURATION:\n");
    _cmd_data_print_cfg(ssdk_cfg);

    dprintf("\n2.DEMO SHELL CONFIGURATION:\n");
    ssdk_cfg_t shell_cfg;
    _cmd_collect_shell_cfg(&shell_cfg);
    _cmd_data_print_cfg(&shell_cfg);

    dprintf("\n3.SSDK FEATURES LIST:\n");

}
