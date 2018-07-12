#include "../halmac_88xx_cfg.h"
#include "halmac_8197f_cfg.h"

/**
 * halmac_mount_api_8197f() - attach functions to function pointer
 * @pHalmac_adapter
 *
 * SD1 internal use
 *
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 */
HALMAC_RET_STATUS
halmac_mount_api_8197f(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	pHalmac_adapter->chip_id = HALMAC_CHIP_ID_8197F;

	pHalmac_adapter->hw_config_info.efuse_size = HALMAC_EFUSE_SIZE_8197F;
	pHalmac_adapter->hw_config_info.eeprom_size = HALMAC_EEPROM_SIZE_8197F;
	pHalmac_adapter->hw_config_info.bt_efuse_size = HALMAC_BT_EFUSE_SIZE_8197F;
	pHalmac_adapter->hw_config_info.cam_entry_num = HALMAC_SECURITY_CAM_ENTRY_NUM_8197F;
	pHalmac_adapter->hw_config_info.txdesc_size = HALMAC_TX_DESC_SIZE_8197F;
	pHalmac_adapter->hw_config_info.rxdesc_size = HALMAC_RX_DESC_SIZE_8197F;
	pHalmac_adapter->hw_config_info.tx_fifo_size = HALMAC_TX_FIFO_SIZE_8197F;
	pHalmac_adapter->hw_config_info.rx_fifo_size = HALMAC_RX_FIFO_SIZE_8197F;
	return HALMAC_RET_SUCCESS;
}

