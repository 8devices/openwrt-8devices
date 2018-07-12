

#ifndef	__EDCATURBOCHECK_H__
#define __EDCATURBOCHECK_H__

void EdcaParaInit(
	struct rtl8192cd_priv *priv
	);

#ifdef WIFI_WMM
VOID
IotEdcaSwitch(
	struct rtl8192cd_priv *priv,
	unsigned char		enable
	);
#endif

BOOLEAN
ChooseIotMainSTA(
	struct rtl8192cd_priv *priv,
	PSTA_INFO_T		pstat
	);

VOID 
IotEngine(
	struct rtl8192cd_priv *priv
	);

#endif

