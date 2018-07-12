#ifndef __HAL88XX_ISR_H__
#define __HAL88XX_ISR_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXIsr.h
	
Abstract:
	Defined HAL 88XX Interrupt Service Routine Related Define & Marco
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-30 Filen            Create.	
--*/












VOID
EnableIMR88XX(
    IN  HAL_PADAPTER    Adapter
);

HAL_IMEM
BOOLEAN
InterruptRecognized88XX(
    IN  HAL_PADAPTER        Adapter,
	IN	PVOID				pContent,
	IN	u4Byte				ContentLen    
);

HAL_IMEM
BOOLEAN
GetInterrupt88XX(
    IN  HAL_PADAPTER        Adapter,
	IN	HAL_INT_TYPE	    intType
);


VOID
AddInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
);


VOID
RemoveInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
);

HAL_IMEM
VOID
DisableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    );

HAL_IMEM
VOID
EnableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    );













#endif  //__HAL88XX_ISR_H__
