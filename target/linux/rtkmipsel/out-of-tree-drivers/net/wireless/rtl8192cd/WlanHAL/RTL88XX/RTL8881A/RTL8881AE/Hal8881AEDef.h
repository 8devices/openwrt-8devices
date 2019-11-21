#ifndef __HAL8881AE_DEF_H__
#define __HAL8881AE_DEF_H__

VOID
C2HPacketHandler_8881A(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
);


#if (BEAMFORMING_SUPPORT == 1)
VOID
SetBeamformEnter8881A(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
);

VOID
SetBeamformLeave8881A(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
);
#endif

#endif  //__HAL8881AE_DEF_H__

