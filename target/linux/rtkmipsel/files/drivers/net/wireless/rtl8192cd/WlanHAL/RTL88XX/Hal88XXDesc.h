#ifndef __HAL88XXDESC_H__
#define __HAL88XXDESC_H__


#define SET_MEM_OP(Dw, Value32, Mask, Shift) \
            (((Dw) & ~((Mask)<<(Shift)))|(((Value32)&(Mask))<<(Shift)))


#if CFG_HAL_CHECK_SWAP

//No Clear First
#define SET_DESC_FIELD_NO_CLR(Dw, Value32, Mask, Shift)   \
            Dw |= (_GET_HAL_DATA(Adapter)->AccessSwapCtrl & HAL_ACCESS_SWAP_MEM) ? \
                    (HAL_cpu_to_le32(((Value32)&(Mask))<<(Shift))): \
                    (((Value32)&(Mask))<<(Shift));

//3 Note: Performance bad, don't use as possible
//Clear first then set
#define SET_DESC_FIELD_CLR(Dw, Value32, Mask, Shift)   \
{ \
	u4Byte lDw = Dw; \
	Dw = (_GET_HAL_DATA(Adapter)->AccessSwapCtrl & HAL_ACCESS_SWAP_MEM) ? \
		(HAL_cpu_to_le32(SET_MEM_OP(HAL_le32_to_cpu(lDw), Value32, Mask, Shift))) : \
		(SET_MEM_OP(lDw, Value32, Mask, Shift)); \
}

#define GET_DESC_FIELD(Dw, Mask, Shift) \
                    ((_GET_HAL_DATA(Adapter)->AccessSwapCtrl & HAL_ACCESS_SWAP_MEM) ? \
                    ((HAL_le32_to_cpu(Dw)>>(Shift)) & (Mask)) : \
                    (((Dw)>>(Shift)) & (Mask)))

#define GET_DESC(val)	((_GET_HAL_DATA(Adapter)->AccessSwapCtrl & HAL_ACCESS_SWAP_MEM) ? HAL_le32_to_cpu(val) : val)
#define SET_DESC(val)	((_GET_HAL_DATA(Adapter)->AccessSwapCtrl & HAL_ACCESS_SWAP_MEM) ? HAL_cpu_to_le32(val) : val)


#else

#define SET_DESC_FIELD_NO_CLR(Dw, Value32, Mask, Shift)   \
                Dw |= (HAL_cpu_to_le32(((Value32)&(Mask))<<(Shift)));


#define SET_DESC_FIELD_CLR(Dw, Value32, Mask, Shift)   \
                Dw = (HAL_cpu_to_le32(SET_MEM_OP(HAL_le32_to_cpu(Dw), Value32, Mask, Shift)));


#define GET_DESC_FIELD(Dw, Mask, Shift) \
                                    ((HAL_le32_to_cpu(Dw)>>(Shift)) & (Mask))

#define GET_DESC(val)	(val)
#define SET_DESC(val)	(val)

#endif

#endif // __HAL88XXDESC_H__
