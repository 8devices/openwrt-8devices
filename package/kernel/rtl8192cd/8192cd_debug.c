/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================

#include "./8192cd_cfg.h"
#include "./8192cd_headers.h"
#include "./8192cd.h"


#define	MAX_ARGC				20
#define	MAX_ARGV				16

#define SPRINTT(buf, fmt, args...) sprintf(buf, "%s"fmt, buf, ##args) 

enum PHYDM_CMD_ID {
	DRV_HELP,
	DRV_DEBUG
};

struct _DRV_COMMAND {
	char name[16];
	unsigned char id;
};


struct _DRV_COMMAND drv_dm_ary[] = {
	{"-h", DRV_HELP},		/*do not move this element to other position*/
	{"drvdbg", DRV_DEBUG} 	
};


#ifdef CONFIG_STATISTICS_INFO
void stats_env_cnt(
	struct rtl8192cd_priv *priv) 
{
	unsigned char i;
	unsigned int fa_cck_idx, fa_ofdm_idx;
	unsigned char str[NUM_STATS_ENV_CNT*10];
	

	#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	if(ODMPTR->original_dig_restore == 0) {
		fa_cck_idx = (ODMPTR->FalseAlmCnt_Acc.Cnt_Cck_fail_1sec[0] + ODMPTR->FalseAlmCnt_Acc.Cnt_Cck_fail_1sec[1]) >> 7;
		fa_cck_idx = (fa_cck_idx > NUM_STATS_ENV_CNT-1) ? (NUM_STATS_ENV_CNT - 1) : fa_cck_idx;
		fa_ofdm_idx = (ODMPTR->FalseAlmCnt_Acc.Cnt_Ofdm_fail_1sec[0] + ODMPTR->FalseAlmCnt_Acc.Cnt_Ofdm_fail_1sec[1]) >> 7;			
		fa_ofdm_idx = (fa_ofdm_idx > NUM_STATS_ENV_CNT-1) ? (NUM_STATS_ENV_CNT - 1) : fa_ofdm_idx;
		
		priv->ext_stats.stats_ofdm_fa[fa_ofdm_idx]++;
		priv->ext_stats.stats_cck_fa[fa_cck_idx]++;		
	} else 
	#endif	//#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	{
		fa_cck_idx = (ODMPTR->FalseAlmCnt.Cnt_Cck_fail >> 7);
		fa_cck_idx = (fa_cck_idx > NUM_STATS_ENV_CNT-1) ? (NUM_STATS_ENV_CNT - 1) : fa_cck_idx;
		fa_ofdm_idx = ODMPTR->FalseAlmCnt.Cnt_Ofdm_fail >> 7;			
		fa_ofdm_idx = (fa_ofdm_idx > NUM_STATS_ENV_CNT-1) ? (NUM_STATS_ENV_CNT - 1) : fa_ofdm_idx;
		
		priv->ext_stats.stats_ofdm_fa[fa_ofdm_idx]++;
		priv->ext_stats.stats_cck_fa[fa_cck_idx]++;
	}

	if((priv->pshare->rf_ft_var.stats_int != 0) && !(priv->up_time % (priv->pshare->rf_ft_var.stats_int*60))) {
		str[0] = '\0';
		for(i=0; i<NUM_STATS_ENV_CNT; i++) {
			SPRINTT(str,"%d ", priv->ext_stats.stats_cck_fa[i]);
			priv->ext_stats.stats_cck_fa[i] = 0;
		}
		DRV_RT_TRACE(priv, DRV_DBG_STATISTICS_INFO, DRV_DBG_SERIOUS, "cck_fa: %s\n", str);

		str[0] = '\0';
		for(i=0; i<NUM_STATS_ENV_CNT; i++) {
			SPRINTT(str,"%d ", priv->ext_stats.stats_ofdm_fa[i]);
			priv->ext_stats.stats_ofdm_fa[i] = 0;
		}
		DRV_RT_TRACE(priv, DRV_DBG_STATISTICS_INFO, DRV_DBG_SERIOUS, "ofdm_fa: %s\n", str);
	}


}
#endif

void drv_init_dbg_setting(
	struct rtl8192cd_priv *priv
)
{
	priv->pshare->rf_ft_var.dbglevel = DRV_DBG_WARNING;
	
	priv->pshare->rf_ft_var.dbgcomp			= \
		//							DRV_DBG_CONN_INFO					|
		//							DRV_DBG_SYS_INFO					|
		//							DRV_DBG_STATISTICS_INFO				|
		0;

}

void drv_debug_trace(
	struct rtl8192cd_priv 	*priv,
	unsigned int			*const dm_value,
	unsigned int			*_used,
	char					*output,
	unsigned int			*_out_len
)
{
	unsigned int			pre_debug_components, one = 1;
	unsigned int			used = *_used;
	unsigned int			out_len = *_out_len;

	pre_debug_components = priv->pshare->rf_ft_var.dbgcomp;

	SPRINTT(output, "\n================================\n");
	if (dm_value[0] == 100) {
		SPRINTT(output, "[Debug Message] Driver Selection\n");
		SPRINTT(output, "================================\n");
		SPRINTT(output, "00. (( %s ))CONN INFO\n", ((priv->pshare->rf_ft_var.dbgcomp & DRV_DBG_CONN_INFO) ? ("V") : (".")));		
		SPRINTT(output, "01. (( %s ))SYSTEM INFO\n", ((priv->pshare->rf_ft_var.dbgcomp & DRV_DBG_SYS_INFO) ? ("V") : (".")));
		SPRINTT(output, "02. (( %s ))STATISTICS INFO\n", ((priv->pshare->rf_ft_var.dbgcomp & DRV_DBG_STATISTICS_INFO) ? ("V") : (".")));
		SPRINTT(output, "================================\n");

	} else if (dm_value[0] == 101) {
		priv->pshare->rf_ft_var.dbgcomp = 0;
		SPRINTT(output, "Disable all debug components\n");
	} else {
		if (dm_value[1] == 1) { /*enable*/
			priv->pshare->rf_ft_var.dbgcomp |= (one << dm_value[0]);
		} else if (dm_value[1] == 2) { /*disable*/
			priv->pshare->rf_ft_var.dbgcomp &= ~(one << dm_value[0]);
		} else
			SPRINTT(output, "[Warning!!!]  1:enable,  2:disable\n");
	}
	SPRINTT(output, "pre-DbgComponents = 0x%x\n", pre_debug_components);
	SPRINTT(output, "Curr-DbgComponents = 0x%x\n", priv->pshare->rf_ft_var.dbgcomp);
	SPRINTT(output, "================================\n");
}


void drv_cmd_parser(
	struct rtl8192cd_priv *priv,
	char			input[][MAX_ARGV],
	unsigned int	input_num,
	char			*output,
	unsigned int	out_len
)
{
	unsigned int used = 0;
	unsigned char id = 0;
	int var1[10] = {0};
	int i, input_idx = 0, drv_ary_size;
	char help[] = "-h";

	//Parsing Cmd ID
	if (input_num) {

		drv_ary_size = sizeof(drv_dm_ary) / sizeof(struct _DRV_COMMAND);
		for (i = 0; i < drv_ary_size; i++) {
			if (strcmp(drv_dm_ary[i].name, input[0]) == 0) {
				id = drv_dm_ary[i].id;
				break;
			}
		}
		if (i == drv_ary_size) {
			SPRINTT(output, "SET, command not found!\n");
			return;
		}
	}

	switch (id) {

	case DRV_HELP:
	{
		SPRINTT(output, "drv cmd ==>\n");
		for (i=0; i < drv_ary_size-2; i++) {

				SPRINTT(output, "  %-5d: %s\n", i, drv_dm_ary[i+2].name);
				/**/
		}
	}
	break;
	case DRV_DEBUG:
	{
		for (i = 0; i < 5; i++) {
			if (input[i + 1]) {
				sscanf(input[i + 1], "%d", &var1[i]);

				/*PHYDM_SNPRINTF((output+used, out_len-used, "new SET, Debug_var[%d]= (( %d ))\n", i , var1[i]));*/
				input_idx++;
			}
		}

		if (input_idx >= 1) {
			/*PHYDM_SNPRINTF((output+used, out_len-used, "odm_debug_comp\n"));*/
			drv_debug_trace(priv, (unsigned int *)var1, &used, output, &out_len);
		}
	}
	break;
	
	default:
		SPRINTT(output, "SET, unknown command!\n");
		break;

	}
}

void
drv_cmd(
		struct rtl8192cd_priv 	*priv,
		unsigned char 			*input,
		char					*output,
		unsigned int			out_len
)
{
		char *token;
		unsigned int	Argc = 0;
		char		Argv[MAX_ARGC][MAX_ARGV];
		unsigned char i;
	
		do {
			token = strsep(&input, ", ");
			if (token) {
				strcpy(Argv[Argc], token);
				Argc++;
			} else
				break;
		} while (Argc < MAX_ARGC);

		for(i=0;i<Argc;i++) {
			panic_printk("%s ", Argv[i]);
		}
		if (Argc == 1)
			Argv[0][strlen(Argv[0]) - 1] = '\0';
	
		drv_cmd_parser(priv, Argv, Argc, output, out_len);
	
		return 0;

}


