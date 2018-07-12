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

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"


#if (CONFIG_ADAPTIVE_SOML)
void
phydm_soml_on_off(
	IN		PVOID		pDM_VOID,
	u8		swch
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

	if (swch == SOML_ON) {
		
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("(( Turn on )) SOML\n"));
#if	(RTL8822B_SUPPORT == 1)
		if (p_dm_odm->SupportICType == ODM_RTL8822B) {
			phydm_somlrxhp_setting(p_dm_odm, TRUE);
		} else
#endif
		if (p_dm_odm->SupportICType == ODM_RTL8197F) {
			ODM_SetBBReg(p_dm_odm, 0x998, BIT(6), swch);
		}
	} else if (swch == SOML_OFF) {
	
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("(( Turn off )) SOML\n"));
#if	(RTL8822B_SUPPORT == 1)
		if (p_dm_odm->SupportICType == ODM_RTL8822B) {
			phydm_somlrxhp_setting(p_dm_odm, FALSE);
		} else 
#endif
		if (p_dm_odm->SupportICType == ODM_RTL8197F) {
			ODM_SetBBReg(p_dm_odm, 0x998, BIT(6), swch);
		}
	}
	p_dm_soml_table->soml_on_off = swch;
}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
void
phydm_adaptive_soml_callback(
	PRT_TIMER		pTimer
)
{
	PADAPTER		Adapter = (PADAPTER)pTimer->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		p_dm_odm = &pHalData->DM_OutSrc;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
#if USE_WORKITEM
	ODM_ScheduleWorkItem(&(p_dm_soml_table->phydm_adaptive_soml_workitem));
#else
	{
		/*dbg_print("phydm_adaptive_soml-phydm_adaptive_soml_callback\n");*/
		phydm_adaptive_soml(p_dm_odm);
	}
#endif
#else
	ODM_ScheduleWorkItem(&(p_dm_soml_table->phydm_adaptive_soml_workitem));
#endif
}

void
phydm_adaptive_soml_workitem_callback(
	IN PVOID            pContext
)
{
	PADAPTER		pAdapter = (PADAPTER)pContext;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	PDM_ODM_T		*p_dm_odm = &(pHalData->DM_OutSrc);

	/*dbg_print("phydm_adaptive_soml-phydm_adaptive_soml_workitem_callback\n");*/
	phydm_adaptive_soml(p_dm_odm);
}

#else

void
phydm_adaptive_soml_callback(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	//struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);
	
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("******SOML_Callback******\n"));
	phydm_adsl(p_dm_odm);

}
#endif

void
phydm_adaptive_soml_timers(
	IN		PVOID		pDM_VOID,
	u8		state
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

	if (state == INIT_SOML_TIMMER) {

		ODM_InitializeTimer(p_dm_odm, &(p_dm_soml_table->phydm_adaptive_soml_timer),
			(void *)phydm_adaptive_soml_callback, NULL, "phydm_adaptive_soml_timer");
	} else if (state == CANCEL_SOML_TIMMER) {

		ODM_CancelTimer(p_dm_odm, &(p_dm_soml_table->phydm_adaptive_soml_timer));
	} else if (state == RELEASE_SOML_TIMMER) {

		ODM_ReleaseTimer(p_dm_odm, &(p_dm_soml_table->phydm_adaptive_soml_timer));
	}
}

void
phydm_soml_debug(
	IN		PVOID		pDM_VOID,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);
	u32 used = *_used;
	u32 out_len = *_out_len;

	if (dm_value[0] == 1) { /*Turn on/off SOML*/
		p_dm_soml_table->soml_select = (u8)dm_value[1];

	} else if (dm_value[0] == 2) { /*training number for SOML*/

		p_dm_soml_table->soml_train_num = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_train_num = ((%d))\n", p_dm_soml_table->soml_train_num));
	} else if (dm_value[0] == 3) { /*training interval for SOML*/

		p_dm_soml_table->soml_intvl = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_intvl = ((%d))\n", p_dm_soml_table->soml_intvl));
	} else if (dm_value[0] == 4) { /*function period for SOML*/

		p_dm_soml_table->soml_period = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_period = ((%d))\n", p_dm_soml_table->soml_period));
	} else if (dm_value[0] == 5) { /*delay_time for SOML*/

		p_dm_soml_table->soml_delay_time= (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_delay_time = ((%d))\n", p_dm_soml_table->soml_delay_time));
	} else if (dm_value[0] == 100) { /*show parameters*/

		PHYDM_SNPRINTF((output + used, out_len - used, "soml_train_num = ((%d))\n", p_dm_soml_table->soml_train_num));
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_intvl = ((%d))\n", p_dm_soml_table->soml_intvl));
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_period = ((%d))\n", p_dm_soml_table->soml_period));
		PHYDM_SNPRINTF((output + used, out_len - used, "soml_delay_time = ((%d))\n", p_dm_soml_table->soml_delay_time));
	}
}

void
phydm_soml_debug_print(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

	u8	i;
	u8	rate_num = 1, rate_ss_shift = 0;
	u32	byte_total = 0;
	u32	ht_reset[HT_RATE_IDX] = {0}, vht_reset[VHT_RATE_IDX] = {0};
	u8	size = sizeof(ht_reset[0]);

	if (p_dm_odm->SupportICType & ODM_IC_4SS)
		rate_num = 4;
	else if (p_dm_odm->SupportICType & ODM_IC_3SS)
		rate_num = 3;
	else if (p_dm_odm->SupportICType & ODM_IC_2SS)
		rate_num = 2;

	if (p_dm_odm->SupportICType == ODM_RTL8197F) {

	for (i = 0; i < rate_num; i++) {
		rate_ss_shift = (i << 3);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("*num_ht_bytes  HT MCS[%d :%d ] = {%d, %d, %d, %d, %d, %d, %d, %d}\n",
			(rate_ss_shift), (rate_ss_shift+7),
			p_dm_soml_table->num_ht_bytes[rate_ss_shift + 0], p_dm_soml_table->num_ht_bytes[rate_ss_shift + 1],
			p_dm_soml_table->num_ht_bytes[rate_ss_shift + 2], p_dm_soml_table->num_ht_bytes[rate_ss_shift + 3],
			p_dm_soml_table->num_ht_bytes[rate_ss_shift + 4], p_dm_soml_table->num_ht_bytes[rate_ss_shift + 5],
			p_dm_soml_table->num_ht_bytes[rate_ss_shift + 6], p_dm_soml_table->num_ht_bytes[rate_ss_shift + 7]));
	}
	
	for (i = 0; i < HT_RATE_IDX; i++) {
		byte_total += p_dm_soml_table->num_ht_bytes[i];
	}
	
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ byte_total = %d ]\n", byte_total));
	ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_ht_bytes, ht_reset, HT_RATE_IDX*size);

	} 
#if	ODM_IC_11AC_SERIES_SUPPORT	
else if (p_dm_odm->SupportICType == ODM_RTL8822B) {

	for (i = 0; i < rate_num; i++) {
		rate_ss_shift = (i << 3);
	
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("*num_ht_bytes  HT MCS[%d :%d ] = {%d, %d, %d, %d, %d, %d, %d, %d}\n",
			(rate_ss_shift), (rate_ss_shift+7),
			p_dm_soml_table->num_vht_bytes[rate_ss_shift + 0], p_dm_soml_table->num_vht_bytes[rate_ss_shift + 1],
			p_dm_soml_table->num_vht_bytes[rate_ss_shift + 2], p_dm_soml_table->num_vht_bytes[rate_ss_shift + 3],
			p_dm_soml_table->num_vht_bytes[rate_ss_shift + 4], p_dm_soml_table->num_vht_bytes[rate_ss_shift + 5],
			p_dm_soml_table->num_vht_bytes[rate_ss_shift + 6], p_dm_soml_table->num_vht_bytes[rate_ss_shift + 7]));
	}
	
	for (i = 0; i < VHT_RATE_IDX; i++) {
		byte_total += p_dm_soml_table->num_ht_bytes[i];
	}
	
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ byte_total = %d ]\n", byte_total));
	ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_vht_bytes, vht_reset, VHT_RATE_IDX*size);
	}
#endif
}

VOID
phydm_soml_statistics(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			on_off_state
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);
	ODM_PHY_DBG_INFO_T		*p_dbg = &(p_dm_odm->PhyDbgInfo);

	u8	i;
	u32	num_bytes_diff;
	
	if (p_dm_odm->SupportICType == ODM_RTL8197F) {
		if (on_off_state == SOML_ON) {
			for (i = 0; i < HT_RATE_IDX; i++) {
				num_bytes_diff = p_dm_soml_table->num_ht_bytes[i] - p_dm_soml_table->pre_num_ht_bytes[i];
				p_dm_soml_table->num_ht_bytes_on[i] += num_bytes_diff;
				p_dm_soml_table->pre_num_ht_bytes[i] = p_dm_soml_table->num_ht_bytes[i];
			}
		} else if (on_off_state == SOML_OFF) {
			for (i = 0; i < HT_RATE_IDX; i++) {
				num_bytes_diff = p_dm_soml_table->num_ht_bytes[i] - p_dm_soml_table->pre_num_ht_bytes[i];
				p_dm_soml_table->num_ht_bytes_off[i] += num_bytes_diff;
				p_dm_soml_table->pre_num_ht_bytes[i] = p_dm_soml_table->num_ht_bytes[i];
			}
		}
	}
#if	ODM_IC_11AC_SERIES_SUPPORT	
	else if (p_dm_odm->SupportICType == ODM_RTL8822B) {
		if (on_off_state == SOML_ON) {
			for (i = 0; i < VHT_RATE_IDX; i++) {
				num_bytes_diff = p_dm_soml_table->num_vht_bytes[i] - p_dm_soml_table->pre_num_vht_bytes[i];
				p_dm_soml_table->num_vht_bytes_on[i] += num_bytes_diff;
				p_dm_soml_table->pre_num_vht_bytes[i] = p_dm_soml_table->num_vht_bytes[i];
			}
		} else if (on_off_state == SOML_OFF) {
			for (i = 0; i < VHT_RATE_IDX; i++) {
				num_bytes_diff = p_dm_soml_table->num_vht_bytes[i] - p_dm_soml_table->pre_num_vht_bytes[i];
				p_dm_soml_table->num_vht_bytes_off[i] += num_bytes_diff;
				p_dm_soml_table->pre_num_vht_bytes[i] = p_dm_soml_table->num_vht_bytes[i];
			}
		}
	}
#endif	
}

void
phydm_adsl(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);
	
	u8	i;
	u8	next_on_off;
	u8	rate_num = 1, rate_ss_shift = 0;
	u32	byte_total_on = 0, byte_total_off = 0;
	u32	ht_reset[HT_RATE_IDX] = {0}, vht_reset[VHT_RATE_IDX] = {0};
	u8	size = sizeof(ht_reset[0]);

	if (p_dm_odm->SupportICType & ODM_IC_4SS)
		rate_num = 4;
	else if (p_dm_odm->SupportICType & ODM_IC_3SS)
		rate_num = 3;
	else if (p_dm_odm->SupportICType & ODM_IC_2SS)
		rate_num = 2;


	if ((p_dm_odm->SupportICType & ODM_ADAPTIVE_SOML_SUPPORT_IC)) {
		if (p_dm_odm->number_active_client == 1) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("number_active_client == 1, RSSI_Min= %d\n", p_dm_odm->RSSI_Min));
			/*Traning state: 0(alt) 1(ori) 2(alt) 3(ori)============================================================*/
			if (p_dm_soml_table->soml_state_cnt < ((p_dm_soml_table->soml_train_num)<<1)) {

				if (p_dm_soml_table->soml_state_cnt == 0) {

					if (p_dm_odm->SupportICType == ODM_RTL8197F) {

						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_ht_bytes, ht_reset, HT_RATE_IDX*size);
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_ht_bytes_on, ht_reset, HT_RATE_IDX*size);
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_ht_bytes_off, ht_reset, HT_RATE_IDX*size);

					}
#if	ODM_IC_11AC_SERIES_SUPPORT					
					else if (p_dm_odm->SupportICType == ODM_RTL8822B) {

						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_vht_bytes, vht_reset, VHT_RATE_IDX*size);
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_vht_bytes_on, vht_reset, VHT_RATE_IDX*size);
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->num_vht_bytes_off, vht_reset, VHT_RATE_IDX*size);
					}
#endif
					p_dm_soml_table->is_soml_method_enable = 1;
					p_dm_soml_table->soml_state_cnt++;
					next_on_off = (p_dm_soml_table->soml_on_off == SOML_ON) ? SOML_ON : SOML_OFF;
					phydm_soml_on_off(p_dm_odm, next_on_off);
					
					ODM_SetTimer(p_dm_odm, &p_dm_soml_table->phydm_adaptive_soml_timer, p_dm_soml_table->soml_delay_time); //delay 40 ms
				} else if ((p_dm_soml_table->soml_state_cnt %2) != 0) {
					p_dm_soml_table->soml_state_cnt++;
					if (p_dm_odm->SupportICType == ODM_RTL8197F) {
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->pre_num_ht_bytes, p_dm_soml_table->num_ht_bytes, HT_RATE_IDX*size);
					}
#if	ODM_IC_11AC_SERIES_SUPPORT					
					else if (p_dm_odm->SupportICType == ODM_RTL8822B) {
						ODM_MoveMemory(p_dm_odm, p_dm_soml_table->pre_num_vht_bytes, p_dm_soml_table->num_vht_bytes, VHT_RATE_IDX*size);
					}
#endif					
					ODM_SetTimer(p_dm_odm, &p_dm_soml_table->phydm_adaptive_soml_timer, p_dm_soml_table->soml_intvl); //turn on/off 150ms
				} else if ((p_dm_soml_table->soml_state_cnt %2) == 0) {
					p_dm_soml_table->soml_state_cnt++;
					phydm_soml_statistics(p_dm_odm, p_dm_soml_table->soml_on_off);
					next_on_off = (p_dm_soml_table->soml_on_off == SOML_ON) ? SOML_OFF : SOML_ON;
					phydm_soml_on_off(p_dm_odm, next_on_off);
					ODM_SetTimer(p_dm_odm, &p_dm_soml_table->phydm_adaptive_soml_timer, p_dm_soml_table->soml_delay_time); //ms
				}

			}
			/*Decision state: 4==============================================================*/
			else {

				p_dm_soml_table->soml_state_cnt = 0;
				phydm_soml_statistics(p_dm_odm, p_dm_soml_table->soml_on_off);

				/* [Search 1st and 2ed rate by counter] */
				if (p_dm_odm->SupportICType == ODM_RTL8197F) {

					for (i = 0; i < rate_num; i++) {
						rate_ss_shift = (i << 3);
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("*num_ht_bytes_on  HT MCS[%d :%d ] = {%d, %d, %d, %d, %d, %d, %d, %d}\n",
							(rate_ss_shift), (rate_ss_shift+7),
							p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 0], p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 1],
							p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 2], p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 3],
							p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 4], p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 5],
							p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 6], p_dm_soml_table->num_ht_bytes_on[rate_ss_shift + 7]));
					}

					for (i = 0; i < rate_num; i++) {
						rate_ss_shift = (i << 3);
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("*num_ht_bytes_off  HT MCS[%d :%d ] = {%d, %d, %d, %d, %d, %d, %d, %d}\n",
							(rate_ss_shift), (rate_ss_shift+7),
							p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 0], p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 1],
							p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 2], p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 3],
							p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 4], p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 5],
							p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 6], p_dm_soml_table->num_ht_bytes_off[rate_ss_shift + 7]));
					}

					for (i = 0; i < HT_RATE_IDX; i++) {

						byte_total_on += p_dm_soml_table->num_ht_bytes_on[i];
						byte_total_off += p_dm_soml_table->num_ht_bytes_off[i];
					}

				}
#if	ODM_IC_11AC_SERIES_SUPPORT				
				else if (p_dm_odm->SupportICType == ODM_RTL8822B) {
					for (i = 0; i < rate_num; i++) {
						rate_ss_shift = 10 * i;
						
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("* num_vht_bytes_on  VHT-%d ss MCS[0:9] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
							(i + 1),
							p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 0], p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 1],
							p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 2], p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 3],
							p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 4], p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 5],
							p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 6], p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 7],
							p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 8], p_dm_soml_table->num_vht_bytes_on[rate_ss_shift + 9]));

					}

					for (i = 0; i < rate_num; i++) {
						rate_ss_shift = 10 * i;
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("* num_vht_bytes_off  VHT-%d ss MCS[0:9] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
							(i + 1),
							p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 0], p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 1],
							p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 2], p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 3],
							p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 4], p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 5],
							p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 6], p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 7],
							p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 8], p_dm_soml_table->num_vht_bytes_off[rate_ss_shift + 9]));
					}
					for (i = 0; i < VHT_RATE_IDX; i++) {
						byte_total_on += p_dm_soml_table->num_vht_bytes_on[i];
						byte_total_off += p_dm_soml_table->num_vht_bytes_off[i];
					}
				}
#endif
				/* [Decision] */
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ byte_total_on = %d ; byte_total_off = %d ]\n", byte_total_on, byte_total_off));
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ Decisoin state ]\n"));


				if (byte_total_on > byte_total_off) {
					next_on_off = SOML_ON;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ byte_total_on > byte_total_off ==> SOML_ON ]\n"));
				}
				else if (byte_total_on < byte_total_off){
					next_on_off = SOML_OFF;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ byte_total_on < byte_total_off ==> SOML_OFF ]\n"));
				} else {
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ stay at soml_last_state ]\n"));
					next_on_off = p_dm_soml_table->soml_last_state;
				}
				 
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[ Final decisoin ] : "));
				phydm_soml_on_off(p_dm_odm, next_on_off);
				p_dm_soml_table->soml_last_state = next_on_off;

					}
		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[number_active_client != 1]\n"));
			phydm_adaptive_soml_reset(p_dm_odm);
			phydm_soml_on_off(p_dm_odm, SOML_ON);
		}
	}
}

void
phydm_adaptive_soml_reset(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T				p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

	p_dm_soml_table->soml_state_cnt = 0;
	p_dm_soml_table->is_soml_method_enable = 0;
	p_dm_soml_table->soml_period = 4;
}

#endif /* end of CONFIG_ADAPTIVE_SOML*/

void
phydm_adaptive_soml_init(
	IN		PVOID		pDM_VOID
)
{
#if (CONFIG_ADAPTIVE_SOML)
	PDM_ODM_T					p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);

	if (!(p_dm_odm->SupportAbility & ODM_BB_ADAPTIVE_SOML)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[Return]   Not Support Adaptive SOML\n"));
		return;
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("phydm_adaptive_soml_init\n"));

	p_dm_soml_table->soml_state_cnt= 0;
	p_dm_soml_table->soml_delay_time = 40;
	p_dm_soml_table->soml_intvl = 150;
	p_dm_soml_table->soml_train_num = 4;
	p_dm_soml_table->is_soml_method_enable = 0;
	p_dm_soml_table->soml_counter = 0;
	p_dm_soml_table->soml_period =  4;
	p_dm_soml_table->soml_select = 0;

	if (p_dm_odm->SupportICType == ODM_RTL8197F)
		ODM_SetBBReg(p_dm_odm, 0x988, BIT(25), 1);
#endif
}

void
phydm_adaptive_soml(
	IN		PVOID		pDM_VOID
)
{
#if (CONFIG_ADAPTIVE_SOML)
	PDM_ODM_T					p_dm_odm = (PDM_ODM_T)pDM_VOID;
	struct _ADAPTIVE_SOML_	*p_dm_soml_table = &(p_dm_odm->dm_soml_table);
	
	if (!(p_dm_odm->SupportAbility & ODM_BB_ADAPTIVE_SOML)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[Return!!!]   Not Support Adaptive SOML Function\n"));
		return;
	}

	if (p_dm_soml_table->soml_counter <  p_dm_soml_table->soml_period) {
		p_dm_soml_table->soml_counter++;
		return;
	} else
		p_dm_soml_table->soml_counter = 0;

	if (p_dm_soml_table->soml_select == 0) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[Adaptive SOML Training !!!]\n"));
	} else if (p_dm_soml_table->soml_select == 1) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[Turn on SOML !!!] Exit from Adaptive SOML Training\n"));
		phydm_soml_on_off(p_dm_odm, SOML_ON);
		phydm_soml_debug_print(p_dm_odm);
		return;
	} else if (p_dm_soml_table->soml_select == 2) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ADAPTIVE_SOML, ODM_DBG_LOUD, ("[Turn off SOML !!!] Exit from Adaptive SOML Training\n"));
		phydm_soml_on_off(p_dm_odm, SOML_OFF);
		phydm_soml_debug_print(p_dm_odm);
		return;
	}

	phydm_adsl(p_dm_odm);

#endif
}
