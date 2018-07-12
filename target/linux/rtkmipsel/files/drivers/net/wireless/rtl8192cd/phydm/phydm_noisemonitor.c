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
#include "mp_precomp.h"
#include "phydm_precomp.h"

//=================================================
// This function is for inband noise test utility only
// To obtain the inband noise level(dbm), do the following.
// 1. disable DIG and Power Saving 
// 2. Set initial gain = 0x1a
// 3. Stop updating idle time pwer report (for driver read)
//	- 0x80c[25]
//
//=================================================

#define Valid_Min				-35
#define Valid_Max			10
#define ValidCnt				5

s2Byte odm_InbandNoise_Monitor_NSeries(PDM_ODM_T	pDM_Odm,u8 bPauseDIG,u8 IGIValue,u32 max_time)
{
	u4Byte				tmp4b;	
	u1Byte				max_rf_path=0,rf_path;	
	u1Byte				reg_c50, reg_c58,valid_done=0;	
	struct noise_level		noise_data;
	u8Byte	start  = 0, func_start = 0,	func_end = 0;

	func_start = ODM_GetCurrentTime(pDM_Odm);
	pDM_Odm->noise_level.noise_all = 0;
	
	if((pDM_Odm->RFType == ODM_1T2R) ||(pDM_Odm->RFType == ODM_2T2R))	
		max_rf_path = 2;
	else
		max_rf_path = 1;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,("odm_DebugControlInbandNoise_Nseries() ==> \n"));

	ODM_Memory_Set(pDM_Odm,&noise_data,0,sizeof(struct noise_level));
	
	// Step 1. Disable DIG && Set initial gain.
	
	if (bPauseDIG)
		odm_PauseDIG(pDM_Odm, PHYDM_PAUSE, PHYDM_PAUSE_LEVEL_1, IGIValue);

	start = ODM_GetCurrentTime(pDM_Odm);
	while(1) {
		
		//Stop updating idle time pwer report (for driver read)
		ODM_SetBBReg(pDM_Odm, rFPGA0_TxGainStage, BIT25, 1);	
		
		//Read Noise Floor Report
		tmp4b = ODM_GetBBReg(pDM_Odm, 0x8f8,bMaskDWord );

		//update idle time pwer report per 5us
		ODM_SetBBReg(pDM_Odm, rFPGA0_TxGainStage, BIT25, 0);

		ODM_delay_us(5);

		noise_data.value[ODM_RF_PATH_A] = (u1Byte)(tmp4b&0xff);		
		noise_data.value[ODM_RF_PATH_B]  = (u1Byte)((tmp4b&0xff00)>>8);

		for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) {
			noise_data.sval[rf_path] = (s1Byte)noise_data.value[rf_path];
			noise_data.sval[rf_path] /= 2;
		}

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,("sval_a = %d, sval_b = %d\n", 
			noise_data.sval[ODM_RF_PATH_A], noise_data.sval[ODM_RF_PATH_B]));

		for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) {
			if (!(noise_data.valid_cnt[rf_path] < ValidCnt) || !(noise_data.sval[rf_path] < Valid_Max && noise_data.sval[rf_path] >= Valid_Min)) {
				continue;
			}

				noise_data.valid_cnt[rf_path]++;
				noise_data.sum[rf_path] += noise_data.sval[rf_path];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("rf_path:%d Valid sval = %d\n", rf_path, noise_data.sval[rf_path]));
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,("Sum of sval = %d, \n", noise_data.sum[rf_path]));
			if (noise_data.valid_cnt[rf_path] == ValidCnt) {
					valid_done++;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("After divided, rf_path:%d,sum = %d\n", rf_path, noise_data.sum[rf_path]));
			}
		}

		if ((valid_done==max_rf_path) || (ODM_GetProgressingTime(pDM_Odm,start) > max_time)) {
			for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) {
				if(noise_data.valid_cnt[rf_path])
					noise_data.sum[rf_path] /= noise_data.valid_cnt[rf_path];
				else
					noise_data.sum[rf_path]  = 0;
			}
			break;
		}
	}
	reg_c50 = (u1Byte)ODM_GetBBReg(pDM_Odm, rOFDM0_XAAGCCore1, bMaskByte0);
	reg_c50 &= ~BIT7;
	pDM_Odm->noise_level.noise[ODM_RF_PATH_A] = (s1Byte)(-110 + reg_c50 + noise_data.sum[ODM_RF_PATH_A]);
	pDM_Odm->noise_level.noise_all += pDM_Odm->noise_level.noise[ODM_RF_PATH_A];
		
	if (max_rf_path == 2) {
		reg_c58 = (u1Byte)ODM_GetBBReg(pDM_Odm, rOFDM0_XBAGCCore1, bMaskByte0);
		reg_c58 &= ~BIT7;
		pDM_Odm->noise_level.noise[ODM_RF_PATH_B] = (s1Byte)(-110 + reg_c58 + noise_data.sum[ODM_RF_PATH_B]);
		pDM_Odm->noise_level.noise_all += pDM_Odm->noise_level.noise[ODM_RF_PATH_B];
	}
	pDM_Odm->noise_level.noise_all /= max_rf_path;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,("noise_a = %d, noise_b = %d\n", 
		pDM_Odm->noise_level.noise[ODM_RF_PATH_A],
		pDM_Odm->noise_level.noise[ODM_RF_PATH_B]));

	//
	// Step 4. Recover the Dig
	//
	if(bPauseDIG)
		odm_PauseDIG(pDM_Odm, PHYDM_RESUME, PHYDM_PAUSE_LEVEL_1, IGIValue);

	func_end = ODM_GetProgressingTime(pDM_Odm,func_start) ;	
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_DebugControlInbandNoise_Nseries() <==\n"));
	return pDM_Odm->noise_level.noise_all;

}

s16
phydm_idle_noise_measurement_ac(
	PDM_ODM_T	pDM_Odm,
	u8	is_pause_dig,
	u8	igi_value,
	u32	max_time
	)
{
	u32				tmp4b;	
	u8				max_rf_path = 0, rf_path;	
	u8				reg_c50, reg_e50, valid_done=0;	
	u64				start  = 0, func_start = 0, func_end = 0;
	struct noise_level	noise_data;
	
	func_start = ODM_GetCurrentTime(pDM_Odm);
	pDM_Odm->noise_level.noise_all = 0;
	
	if ((pDM_Odm->RFType == ODM_1T2R) ||(pDM_Odm->RFType == ODM_2T2R))	
		max_rf_path = 2;
	else
		max_rf_path = 1;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("phydm_idle_noise_measurement_ac==>\n"));

	ODM_Memory_Set(pDM_Odm, &noise_data, 0, sizeof(struct noise_level));
	
	/*Step 1. Disable DIG && Set initial gain.*/
	
	if (is_pause_dig)
		odm_PauseDIG(pDM_Odm, PHYDM_PAUSE, PHYDM_PAUSE_LEVEL_1, igi_value);

	/*Step 2. Get noise power level*/
	start = ODM_GetCurrentTime(pDM_Odm);

	while(1) {
		
		/*Stop updating idle time pwer report (for driver read)*/
		ODM_SetBBReg(pDM_Odm, 0x9e4, BIT(30), 0x1);	
		
		/*Read Noise Floor Report*/
		tmp4b = ODM_GetBBReg(pDM_Odm, 0xff0, bMaskDWord);
		
		/*update idle time pwer report per 5us*/
		ODM_SetBBReg(pDM_Odm, 0x9e4, BIT(30), 0x0);

		ODM_delay_us(5);

		noise_data.value[ODM_RF_PATH_A] = (u8)(tmp4b & 0xff);		
		noise_data.value[ODM_RF_PATH_B]  = (u8)((tmp4b & 0xff00) >> 8);
	
		for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) {
			noise_data.sval[rf_path] = (s8)noise_data.value[rf_path];
			noise_data.sval[rf_path] = noise_data.sval[rf_path] >> 1;
		}

		for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) {
			if (!(noise_data.valid_cnt[rf_path] < ValidCnt) || !(noise_data.sval[rf_path] < Valid_Max && noise_data.sval[rf_path] >= Valid_Min)) {
				continue;
			}

				noise_data.valid_cnt[rf_path]++;
				noise_data.sum[rf_path] += noise_data.sval[rf_path];
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("rf_path:%d Valid sval = %d\n", rf_path, noise_data.sval[rf_path]));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("Sum of sval = %d,\n", noise_data.sum[rf_path]));
			if (noise_data.valid_cnt[rf_path] == ValidCnt) {
					valid_done++;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("After divided, rf_path:%d,sum = %d\n", rf_path, noise_data.sum[rf_path]));
			}
		}

		if ((valid_done == max_rf_path) || (ODM_GetProgressingTime(pDM_Odm, start) > max_time)) {
			for (rf_path = ODM_RF_PATH_A; rf_path < max_rf_path; rf_path++) { 
				if (noise_data.valid_cnt[rf_path])
					noise_data.sum[rf_path] /= noise_data.valid_cnt[rf_path];
				else
					noise_data.sum[rf_path]  = 0;

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,("Path%d, avg of sval = %d\n", rf_path, noise_data.sum[rf_path]));
			}
			break;
		}
	}
	reg_c50 = (u8)ODM_GetBBReg(pDM_Odm, 0xc50, bMaskByte0);
	reg_c50 &= ~ BIT(7);
	pDM_Odm->noise_level.noise[ODM_RF_PATH_A] = (s8)(-110 + reg_c50 + noise_data.sum[ODM_RF_PATH_A]);
	pDM_Odm->noise_level.noise_all += pDM_Odm->noise_level.noise[ODM_RF_PATH_A];
		
	if (max_rf_path == 2) {
		reg_e50 = (u8)ODM_GetBBReg(pDM_Odm, 0xe50, bMaskByte0);
		reg_e50 &= ~ BIT(7);
		pDM_Odm->noise_level.noise[ODM_RF_PATH_B] = (s8)(-110 + reg_e50 + noise_data.sum[ODM_RF_PATH_B]);
		pDM_Odm->noise_level.noise_all += pDM_Odm->noise_level.noise[ODM_RF_PATH_B];
	}
	pDM_Odm->noise_level.noise_all /= max_rf_path;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_COMMON, ODM_DBG_LOUD,
		("noise_a = %d, noise_b = %d, noise_all = %d\n", 
		pDM_Odm->noise_level.noise[ODM_RF_PATH_A],
		pDM_Odm->noise_level.noise[ODM_RF_PATH_B],
		pDM_Odm->noise_level.noise_all));

	/*Step 3. Recover the Dig*/
	if (is_pause_dig)
		odm_PauseDIG(pDM_Odm, PHYDM_RESUME, PHYDM_PAUSE_LEVEL_1, igi_value);
	func_end = ODM_GetProgressingTime(pDM_Odm, func_start) ;	
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("phydm_idle_noise_measurement_ac<==\n"));
	return pDM_Odm->noise_level.noise_all;

}

s2Byte	
odm_InbandNoise_Monitor_ACSeries(PDM_ODM_T	pDM_Odm, u8 bPauseDIG, u8 IGIValue, u32 max_time
	)
{
	s4Byte          rxi_buf_anta, rxq_buf_anta; /*rxi_buf_antb, rxq_buf_antb;*/
	s4Byte	        value32, pwdb_A = 0, sval, noise, sum = 0;
	BOOLEAN	        pd_flag;
	u1Byte		valid_cnt = 0;
	u8Byte	start = 0, func_start = 0, func_end = 0;

	if (pDM_Odm->SupportICType & ODM_RTL8822B)
		return phydm_idle_noise_measurement_ac(pDM_Odm, bPauseDIG, IGIValue, max_time);

	if (!(pDM_Odm->SupportICType & (ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8814A)))
		return 0;

	func_start = ODM_GetCurrentTime(pDM_Odm);
	pDM_Odm->noise_level.noise_all = 0;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_InbandNoise_Monitor_ACSeries() ==>\n"));
	
	/* Step 1. Disable DIG && Set initial gain. */
	if (bPauseDIG)
		odm_PauseDIG(pDM_Odm, PHYDM_PAUSE, PHYDM_PAUSE_LEVEL_1, IGIValue);

	/* Step 2. Disable all power save for read registers */
	/*dcmd_DebugControlPowerSave(pAdapter, PSDisable); */

	/* Step 3. Get noise power level */
	start = ODM_GetCurrentTime(pDM_Odm);

	/* Step 4. Get noise power level */
	while (1) {
		/*Set IGI=0x1C */
		ODM_Write_DIG(pDM_Odm, 0x1C);
		/*stop CK320&CK88 */
		ODM_SetBBReg(pDM_Odm, 0x8B4, BIT6, 1);
		/*Read Path-A */
		ODM_SetBBReg(pDM_Odm, 0x8FC, bMaskDWord, 0x200); /*set debug port*/
		value32 = ODM_GetBBReg(pDM_Odm, 0xFA0, bMaskDWord); /*read debug port*/
		
		rxi_buf_anta = (value32 & 0xFFC00) >> 10; /*rxi_buf_anta=RegFA0[19:10]*/
		rxq_buf_anta = value32 & 0x3FF; /*rxq_buf_anta=RegFA0[19:10]*/

		pd_flag = (BOOLEAN) ((value32 & BIT31) >> 31);

		/*Not in packet detection period or Tx state */
		if ((!pd_flag) || (rxi_buf_anta != 0x200)) {
			/*sign conversion*/
			rxi_buf_anta = ODM_SignConversion(rxi_buf_anta, 10);
			rxq_buf_anta = ODM_SignConversion(rxq_buf_anta, 10);

			pwdb_A = ODM_PWdB_Conversion(rxi_buf_anta * rxi_buf_anta + rxq_buf_anta * rxq_buf_anta, 20, 18); /*S(10,9)*S(10,9)=S(20,18)*/

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("pwdb_A= %d dB, rxi_buf_anta= 0x%x, rxq_buf_anta= 0x%x\n", pwdb_A, rxi_buf_anta & 0x3FF, rxq_buf_anta & 0x3FF));
		}
		/*Start CK320&CK88*/
		ODM_SetBBReg(pDM_Odm, 0x8B4, BIT6, 0);
		/*BB Reset*/
		ODM_Write1Byte(pDM_Odm, 0x02, ODM_Read1Byte(pDM_Odm, 0x02) & (~BIT0));
		ODM_Write1Byte(pDM_Odm, 0x02, ODM_Read1Byte(pDM_Odm, 0x02) | BIT0);
		/*PMAC Reset*/
		ODM_Write1Byte(pDM_Odm, 0xB03, ODM_Read1Byte(pDM_Odm, 0xB03) & (~BIT0));
		ODM_Write1Byte(pDM_Odm, 0xB03, ODM_Read1Byte(pDM_Odm, 0xB03) | BIT0);
		/*CCK Reset*/
		if (ODM_Read1Byte(pDM_Odm, 0x80B) & BIT4) {
			ODM_Write1Byte(pDM_Odm, 0x80B, ODM_Read1Byte(pDM_Odm, 0x80B) & (~BIT4));
			ODM_Write1Byte(pDM_Odm, 0x80B, ODM_Read1Byte(pDM_Odm, 0x80B) | BIT4);		
		}

		sval = pwdb_A;

		if (sval < 0 && sval >= -27) {
			if (valid_cnt < ValidCnt) {
				valid_cnt++;
				sum += sval;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("Valid sval = %d\n", sval));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("Sum of sval = %d,\n", sum));
				if ((valid_cnt >= ValidCnt) || (ODM_GetProgressingTime(pDM_Odm, start) > max_time)) {
					sum /= valid_cnt;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("After divided, sum = %d\n", sum)); 
					break;
				}
			}
		}
	}

	/*ADC backoff is 12dB,*/ 
	/*Ptarget=0x1C-110=-82dBm*/
	noise = sum + 12 + 0x1C - 110; 
	
	/*Offset*/
	noise = noise - 3;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("noise = %d\n", noise));
	pDM_Odm->noise_level.noise_all = (s2Byte)noise;

	/* Step 4. Recover the Dig*/
	if (bPauseDIG)
		odm_PauseDIG(pDM_Odm, PHYDM_RESUME, PHYDM_PAUSE_LEVEL_1, IGIValue);
	
	func_end = ODM_GetProgressingTime(pDM_Odm, func_start);
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("odm_InbandNoise_Monitor_ACSeries() <==\n"));

	return pDM_Odm->noise_level.noise_all;
}



s2Byte
ODM_InbandNoise_Monitor(PVOID pDM_VOID, u8 bPauseDIG, u8 IGIValue, u32 max_time)
{

	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if (pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
		return odm_InbandNoise_Monitor_ACSeries(pDM_Odm, bPauseDIG, IGIValue, max_time);
	else
		return odm_InbandNoise_Monitor_NSeries(pDM_Odm, bPauseDIG, IGIValue, max_time);
}

