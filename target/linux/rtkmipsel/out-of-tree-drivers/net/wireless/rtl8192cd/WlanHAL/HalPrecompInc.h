#ifndef __INC_PRECOMPINC_H
#define __INC_PRECOMPINC_H

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
//HAL Shared with Driver
#include "StatusCode.h"
#include "HalDbgCmd.h"

#ifdef CONFIG_WLAN_MACHAL_API
#include "HalMacAPI.h"
#endif

//MAC function support
#include "HalMacFunc.h"

//Prototype
#include "HalDef.h"

//MAC Header provided by SD1 HWSD
#include "HalHWCfg.h"
#include "HalComTXDesc.h"
#include "HalComRXDesc.h"
//#include "HalComBit.h"
//#include "HalComReg.h"
#include "HalComPhyBit.h"
#include "HalComPhyReg.h"
//#include "HalRxDescAp.h"
//#include "HalTxDescAp.h"
#include "halmac_api.h"

//#include "halmac_reg2.h"
//#include "halmac_bit2.h"
//#include "halmac_rx_desc_ap.h"
#include "halmac_tx_desc_ap.h"




//Instance
#include "HalCommon.h"

 


#if IS_RTL88XX_GENERATION
#include "Hal88XXPwrSeqCmd.h"
#include "Hal88XXReg.h"
#include "Hal88XXDesc.h"
#include "Hal88XXTxDesc.h"
#include "Hal88XXRxDesc.h"
#include "Hal88XXFirmware.h"
#include "Hal88XXIsr.h"
#include "Hal88XXDebug.h"
#include "Hal88XXPhyCfg.h"
#include "Hal88XXDM.h"

#if IS_RTL8881A_SERIES
#include "Hal8881APwrSeqCmd.h"
#include "Hal8881ADef.h"
#include "Hal8881APhyCfg.h"
#endif

#if IS_RTL8192E_SERIES
#include "Hal8192EPwrSeqCmd.h"
#include "Hal8192EDef.h"
#include "Hal8192EPhyCfg.h"
#endif

#if IS_RTL8197F_SERIES
#include "RTL88XX/RTL8197F/Hal8197FPwrSeqCmd.h"
#include "RTL88XX/RTL8197F/Hal8197FDef.h"
#include "RTL88XX/RTL8197F/Hal8197FPhyCfg.h"
#endif //#if IS_RTL8197F_SERIES

#if IS_RTL8822B_SERIES
#include "RTL88XX/RTL8822B/Hal8822BPwrSeqCmd.h"
#include "RTL88XX/RTL8822B/Hal8822BDef.h"
#include "RTL88XX/RTL8822B/Hal8822BPhyCfg.h"
#include "RTL88XX/RTL8822B/RTL8822BE/Hal8822BEDef.h"
#endif //#if IS_RTL8822B_SERIES


#if IS_RTL8814A_SERIES
#include "Hal8814APwrSeqCmd.h"
#include "Hal8814ADef.h"
#include "Hal8814APhyCfg.h"
#include "Hal8814AFirmware.h"

#endif

#if IS_EXIST_RTL8192EE
#include "Hal8192EEDef.h"
#endif

#if IS_EXIST_RTL8814AE
#include "Hal8814AEDef.h"
#endif

#include "Hal88XXDef.h"

#endif  //IS_RTL88XX_GENERATION

#else  

//HAL Shared with Driver
#include "Include/StatusCode.h"
#include "HalDbgCmd.h"


#ifdef CONFIG_WLAN_MACHAL_API
#include "HalMacAPI.h"
#endif

//MAC function support
#include "HalMacFunc.h"

//Prototype
#include "HalDef.h"

//MAC Header provided by SD1 HWSD
#include "HalHeader/HalHWCfg.h"
#include "HalHeader/HalComTXDesc.h"
#include "HalHeader/HalComRXDesc.h"
//#include "HalHeader/HalComBit.h"
//#include "HalHeader/HalComReg.h"
#include "HalMac88XX/halmac_reg2.h"
#include "HalMac88XX/halmac_bit2.h"


#include "HalHeader/HalComPhyBit.h"
#include "HalHeader/HalComPhyReg.h"
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalHeader/HalRxDescAp.h"
#include "HalHeader/HalTxDescAp.h"
#endif


//Instance
#include "HalCommon.h"

#if IS_RTL88XX_GENERATION

#include "RTL88XX/Hal88XXPwrSeqCmd.h"
#include "RTL88XX/Hal88XXReg.h"
#include "RTL88XX/Hal88XXDesc.h"
#include "RTL88XX/Hal88XXTxDesc.h"
#include "RTL88XX/Hal88XXRxDesc.h"
#include "RTL88XX/Hal88XXFirmware.h"
#include "RTL88XX/Hal88XXIsr.h"
#include "RTL88XX/Hal88XXDebug.h"
#include "RTL88XX/Hal88XXPhyCfg.h"
#include "RTL88XX/Hal88XXDM.h"


#if IS_RTL8881A_SERIES
#include "RTL88XX/RTL8881A/Hal8881APwrSeqCmd.h"
#include "RTL88XX/RTL8881A/Hal8881ADef.h"
#include "RTL88XX/RTL8881A/Hal8881APhyCfg.h"
#endif

#if IS_RTL8192E_SERIES
#include "RTL88XX/RTL8192E/Hal8192EPwrSeqCmd.h"
#include "RTL88XX/RTL8192E/Hal8192EDef.h"
#include "RTL88XX/RTL8192E/Hal8192EPhyCfg.h"
#endif

#if IS_RTL8814A_SERIES
#include "RTL88XX/RTL8814A/Hal8814APwrSeqCmd.h"
#include "RTL88XX/RTL8814A/Hal8814ADef.h"
#include "RTL88XX/RTL8814A/Hal8814APhyCfg.h"
#include "RTL88XX/RTL8814A/Hal8814AFirmware.h"
#endif

#if IS_RTL8822B_SERIES
#include "RTL88XX/RTL8822B/Hal8822BPwrSeqCmd.h"
#include "RTL88XX/RTL8822B/Hal8822BDef.h"
#include "RTL88XX/RTL8822B/Hal8822BPhyCfg.h"
#include "RTL88XX/RTL8822B/RTL8822BE/Hal8822BEDef.h"
#endif //#if IS_RTL8822B_SERIES


#if IS_RTL8197F_SERIES
#include "RTL88XX/RTL8197F/Hal8197FPwrSeqCmd.h"
#include "RTL88XX/RTL8197F/Hal8197FDef.h"
#include "RTL88XX/RTL8197F/Hal8197FPhyCfg.h"
#endif //#if IS_RTL8197F_SERIES

#if IS_EXIST_RTL8192EE
#include "RTL88XX/RTL8192E/RTL8192EE/Hal8192EEDef.h"
#endif

#if IS_EXIST_RTL8814AE
#include "RTL88XX/RTL8814A/RTL8814AE/Hal8814AEDef.h"
#endif


#include "RTL88XX/Hal88XXDef.h"
#endif  //IS_RTL88XX_GENERATION

#endif  //ECOS

#endif  //#ifndef __INC_PRECOMPINC_H
