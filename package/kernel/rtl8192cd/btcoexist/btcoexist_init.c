#include "./btcoexist_init.h"
#include "../8192cd.h"


static u32 rtl8197f_hal_init(BT_COEX_HAL_ADAPTER *padapter)
{
	rtw_btcoex_HAL_Initialize(padapter, _FALSE);
}

static u32 rtl8197f_hal_deinit(BT_COEX_HAL_ADAPTER *padapter)
{
	printk("%s() <====\n", __FUNCTION__);

	return _SUCCESS;

}

void rtl8197f_set_hw_reg(PADAPTER padapter, u8 variable, u8 *pval)
{
/*	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);


	switch (variable) {
	case HW_VAR_SET_RPWM:
		break;
	case HW_VAR_AMPDU_MAX_TIME: {
		u8	maxRate = *(u8 *)val;

		if (maxRate > MGN_VHT1SS_MCS9)
			rtw_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x70);
		else if (IS_HARDWARE_TYPE_8812(Adapter))
			rtw_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0xe0);
		else
			rtw_write8(Adapter, REG_AMPDU_MAX_TIME_8812, 0x5e);
	}
	break;
	case HW_VAR_PCIE_STOP_TX_DMA:
		rtw_write16(Adapter, REG_PCIE_CTRL_REG, 0xff00);
		break;

	case HW_VAR_DBI:
		{
		u16 *pCmd;

		pCmd = (u16 *)val;
		hal_dbi_write_8812ae(Adapter, pCmd[0], (u8)pCmd[1]);

		break;
		}
	case HW_VAR_MDIO:
		{
		u16 *pCmd;

		pCmd = (u16 *)val;
		hal_mdio_write_8812ae(Adapter, (u8)pCmd[0], pCmd[1]);
		break;
		}
	default:
		SetHwReg8812A(Adapter, variable, val);
		break;
	}
*/

}


void rtl8197f_get_adapter_info(BT_COEX_HAL_ADAPTER *padapter)
{
	/* Read all content in Efuse/EEPROM. */

	/*owing to 97F did not have efuse for bt-coexist info, we have to hard cord here*/
	padapter->pshare->bt_coex_comm_data.EEPROMBluetoothCoexist	= 1;
	padapter->pshare->bt_coex_comm_data.EEPROMBluetoothType		= BT_RTL8761;
	padapter->pshare->bt_coex_comm_data.EEPROMBluetoothAntNum	= Ant_x2;
	padapter->pshare->bt_coex_comm_data.bt_coex_status.btAntisolation	= 0;
}


void rtl8197f_set_bt_coexist_hal_ops(BT_COEX_HAL_ADAPTER *padapter)
{
	struct bt_coexist_hal_ops *bt_coex_hal_func = &padapter->pshare->bt_coex_comm_data.bt_coex_hal_func;

	/*hal init & deinit*/
	bt_coex_hal_func->hal_init = &rtl8197f_hal_init;
	bt_coex_hal_func->hal_deinit = &rtl8197f_hal_deinit;

	bt_coex_hal_func->set_hw_reg_handler = &rtl8197f_set_hw_reg;

	bt_coex_hal_func->get_adapter_info = &rtl8197f_get_adapter_info;
}

u8 bt_coexist_set_hal_ops(BT_COEX_HAL_ADAPTER *padapter)
{
	
/* you can add new chip's hal_ops here */

#if defined(CONFIG_WLAN_HAL_8197F) && (CONFIG_BT_COEXIST_8197F_8761)
/* 20180626 only verified the 97F+8761*/
	if (GET_CHIP_VER(padapter) == VERSION_8197F){
		rtl8197f_set_bt_coexist_hal_ops(padapter);
	}
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)
	//copy 8197F setting
	if ((GET_CHIP_VER(padapter) == VERSION_8822B)){
		//rtl8812ae_set_bt_coexist_hal_ops(padapter);
	}
#endif


}

void bt_coexist_load_param(BT_COEX_HAL_ADAPTER *padapter)
{
	bool status = _SUCCESS;

	struct bt_coexist_common_data *registry_par;
	registry_par = &(padapter->pshare->bt_coex_comm_data);
	
	registry_par->btcoex = padapter->pshare->rf_ft_var.rtw_btcoex_enable;
	registry_par->bt_iso = padapter->pshare->rf_ft_var.rtw_bt_iso;
	registry_par->bt_sco = padapter->pshare->rf_ft_var.rtw_bt_sco;
	registry_par->bt_ampdu = padapter->pshare->rf_ft_var.rtw_bt_ampdu;
	registry_par->ant_num = padapter->pshare->rf_ft_var.rtw_ant_num;

	
}
//extern void rtw_btcoex_init_socket(BT_COEX_HAL_ADAPTER *padapter);
bool bt_coexist_init_socket(BT_COEX_HAL_ADAPTER *padapter)
{
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	if (IS_BT_COEXIST_AVALIABLE(padapter)==TRUE ) {
		rtw_btcoex_init_socket(padapter);
		padapter->pshare->bt_coex_comm_data.coex_info.BtMgnt.ExtConfig.HCIExtensionVer = 0x04;
		rtw_btcoex_SetHciVersion(padapter, 0x04);
	} else
		printk("CONFIG_BT_COEXIST: VIRTUAL_ADAPTER\n");
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
}

void bt_coexist_get_efuse_eeprom_data(BT_COEX_HAL_ADAPTER *padapter)
{
	struct bt_coexist_hal_ops *bt_coex_hal_func = &padapter->pshare->bt_coex_comm_data.bt_coex_hal_func;
	
	bt_coex_hal_func->get_adapter_info(padapter);	
}

int rtl8192cd_bt_coexist_func_init(BT_COEX_HAL_ADAPTER *padapter)
{
	//step1
	bt_coexist_load_param(padapter);

	//step2
	bt_coexist_set_hal_ops(padapter);
	
	//step3
	bt_coexist_get_efuse_eeprom_data(padapter);

	//step4
	bt_coexist_init_socket(padapter);
	
	//step5
	rtw_btcoex_Initialize(padapter);
	
	return 0;
}
