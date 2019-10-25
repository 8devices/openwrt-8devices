//============================================================
// File Name: Type_def.h
//
// Description:
//
//============================================================


#ifndef	__TYPE_DEF_H__
#define __TYPE_DEF_H__

typedef void				VOID,*PVOID;

#ifndef __OSK__
typedef unsigned char		BOOLEAN,*PBOOLEAN;
#endif

typedef unsigned char		u1Byte,*pu1Byte;
typedef unsigned short		u2Byte,*pu2Byte;
typedef unsigned int		u4Byte,*pu4Byte;
typedef unsigned long long	u8Byte,*pu8Byte;

/* In ARM platform, system would use the type -- "char" as "unsigned char"*/
typedef signed char		s1Byte,*ps1Byte;
typedef signed short		s2Byte,*ps2Byte;
typedef signed int			s4Byte,*ps4Byte;
typedef signed long long		s8Byte,*ps8Byte;

typedef unsigned int		UINT;
typedef signed int			INT;

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(UINT8))
typedef unsigned int		UINT32,*pUINT32;
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef signed char			INT8;
typedef signed short		INT16;
typedef signed int			INT32;
typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif

#ifndef BIT0
#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400
#define BIT11		0x00000800
#define BIT12		0x00001000
#define BIT13		0x00002000
#define BIT14		0x00004000
#define BIT15		0x00008000
#define BIT16		0x00010000
#define BIT17		0x00020000
#define BIT18		0x00040000
#define BIT19		0x00080000
#define BIT20		0x00100000
#define BIT21		0x00200000
#define BIT22		0x00400000
#define BIT23		0x00800000
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000

#endif

#ifndef BIT32
#ifndef UINT64_C
#define UINT64_C(v)  (v ## ULL)
#endif
#define BIT32	UINT64_C(0x0000000100000000)
#define BIT33	UINT64_C(0x0000000200000000)
#define BIT34	UINT64_C(0x0000000400000000)
#define BIT35	UINT64_C(0x0000000800000000)
#define BIT36	UINT64_C(0x0000001000000000)
#define BIT37	UINT64_C(0x0000002000000000)
#define BIT38	UINT64_C(0x0000004000000000)
#define BIT39	UINT64_C(0x0000008000000000)
#define BIT40	UINT64_C(0x0000010000000000)
#define BIT41	UINT64_C(0x0000020000000000)
#define BIT42	UINT64_C(0x0000040000000000)
#define BIT43	UINT64_C(0x0000080000000000)
#define BIT44	UINT64_C(0x0000100000000000)
#define BIT45	UINT64_C(0x0000200000000000)
#define BIT46	UINT64_C(0x0000400000000000)
#define BIT47	UINT64_C(0x0000800000000000)
#define BIT48	UINT64_C(0x0001000000000000)
#define BIT49	UINT64_C(0x0002000000000000)
#define BIT50	UINT64_C(0x0004000000000000)
#define BIT51	UINT64_C(0x0008000000000000)
#define BIT52	UINT64_C(0x0010000000000000)
#define BIT53	UINT64_C(0x0020000000000000)
#define BIT54	UINT64_C(0x0040000000000000)
#define BIT55	UINT64_C(0x0080000000000000)
#define BIT56	UINT64_C(0x0100000000000000)
#define BIT57	UINT64_C(0x0200000000000000)
#define BIT58	UINT64_C(0x0400000000000000)
#define BIT59	UINT64_C(0x0800000000000000)
#define BIT60	UINT64_C(0x1000000000000000)
#define BIT61	UINT64_C(0x2000000000000000)
#define BIT62	UINT64_C(0x4000000000000000)
#define BIT63	UINT64_C(0x8000000000000000)
#endif

//	Example:
//		BIT_LEN_MASK_32(0) => 0x00000000
//		BIT_LEN_MASK_32(1) => 0x00000001
//		BIT_LEN_MASK_32(2) => 0x00000003
//		BIT_LEN_MASK_32(32) => 0xFFFFFFFF
//
#define BIT_LEN_MASK_32(__BitLen) \
	(0xFFFFFFFF >> (32 - (__BitLen)))

#define BIT_LEN_MASK_8(__BitLen) \
		(0xFF >> (8 - (__BitLen)))

//
// Byte Swapping routine.
//
#define EF1Byte	
#define EF2Byte 	le16_to_cpu
#define EF4Byte		le32_to_cpu

#define LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
	(EF1Byte(*((u1Byte *)(__pStart))))

//
//	Description:
//		Return 4-byte value in host byte ordering from
//		4-byte pointer in litten-endian system.
//
#define LE_P4BYTE_TO_HOST_4BYTE(__pStart) \
	(EF4Byte(*((u4Byte *)(__pStart))))
	

//
//	Description:
//		Translate subfield (continuous bits in little-endian) of 4-byte value in litten byte to
//		4-byte value in host byte ordering.
//
#define LE_BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P4BYTE_TO_HOST_4BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_32(__BitLen) \
	)

#define LE_BITS_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P1BYTE_TO_HOST_1BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_8(__BitLen) \
	)

// PF3 Tx beamforming
#ifndef UNALIGNED
#define UNALIGNED
#endif

#define FRAME_OFFSET_FRAME_CONTROL		0
#define FRAME_OFFSET_DURATION			2
#define FRAME_OFFSET_ADDRESS1			4
#define FRAME_OFFSET_ADDRESS2			10
#define FRAME_OFFSET_ADDRESS3			16
#define FRAME_OFFSET_SEQUENCE			22
#define FRAME_OFFSET_ADDRESS4			24

#define WriteEF1Byte(_ptr, _val)	(*((pu1Byte)(_ptr)))=EF1Byte(_val)
#define WriteEF2Byte(_ptr, _val)	(*((UNALIGNED pu2Byte)(_ptr)))=EF2Byte(_val)
#define WriteEF4Byte(_ptr, _val)	(*((UNALIGNED pu4Byte)(_ptr)))=EF4Byte(_val)	


//	Example:
//		BIT_LEN_MASK_32(0) => 0x00000000
//		BIT_LEN_MASK_32(1) => 0x00000001
//		BIT_LEN_MASK_32(2) => 0x00000003
//		BIT_LEN_MASK_32(32) => 0xFFFFFFFF
//
#define BIT_LEN_MASK_32(__BitLen) \
	(0xFFFFFFFF >> (32 - (__BitLen)))
//
//	Example:
//		BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003
//		BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000
//
#define BIT_OFFSET_LEN_MASK_32(__BitOffset, __BitLen) \
	(BIT_LEN_MASK_32(__BitLen) << (__BitOffset)) 

#define LE_BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P4BYTE_TO_HOST_4BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_32(__BitLen) \
	)

//
//	Description:
//		Mask subfield (continuous bits in little-endian) of 4-byte value in litten byte oredering  
//		and return the result in 4-byte value in host byte ordering.
//
#define LE_BITS_CLEARED_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		LE_P4BYTE_TO_HOST_4BYTE(__pStart) \
		& \
		( ~BIT_OFFSET_LEN_MASK_32(__BitOffset, __BitLen) ) \
	)

//
//	Description:
//		Set subfield of little-endian 4-byte value to specified value.	
//
#define SET_BITS_TO_LE_4BYTE(__pStart, __BitOffset, __BitLen, __Value) \
	*((UNALIGNED pu4Byte)(__pStart)) = \
		EF4Byte( \
			LE_BITS_CLEARED_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
			| \
			( (((u4Byte)__Value) & BIT_LEN_MASK_32(__BitLen)) << (__BitOffset) ) \
		);

		
#define BIT_LEN_MASK_16(__BitLen) \
		(0xFFFF >> (16 - (__BitLen)))
		
#define BIT_OFFSET_LEN_MASK_16(__BitOffset, __BitLen) \
	(BIT_LEN_MASK_16(__BitLen) << (__BitOffset))
	
#define LE_P2BYTE_TO_HOST_2BYTE(__pStart) \
	(EF2Byte(*((UNALIGNED pu2Byte)(__pStart))))
	
#define LE_BITS_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P2BYTE_TO_HOST_2BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_16(__BitLen) \
	)
	
#define LE_BITS_CLEARED_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		LE_P2BYTE_TO_HOST_2BYTE(__pStart) \
		& \
		( ~BIT_OFFSET_LEN_MASK_16(__BitOffset, __BitLen) ) \
	)

#define SET_BITS_TO_LE_2BYTE(__pStart, __BitOffset, __BitLen, __Value) \
	*((UNALIGNED pu2Byte)(__pStart)) = \
		EF2Byte( \
			LE_BITS_CLEARED_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
			| \
			( (((u2Byte)__Value) & BIT_LEN_MASK_16(__BitLen)) << (__BitOffset) ) \
		);
					
#define BIT_LEN_MASK_8(__BitLen) \
		(0xFF >> (8 - (__BitLen)))

#define BIT_OFFSET_LEN_MASK_8(__BitOffset, __BitLen) \
	(BIT_LEN_MASK_8(__BitLen) << (__BitOffset))

/*
#define LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
	(EF1Byte(*((pu1Byte)(__pStart))))
*/
#define LE_BITS_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		( LE_P1BYTE_TO_HOST_1BYTE(__pStart) >> (__BitOffset) ) \
		& \
		BIT_LEN_MASK_8(__BitLen) \
	)

#define LE_BITS_CLEARED_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
	( \
		LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
		& \
		( ~BIT_OFFSET_LEN_MASK_8(__BitOffset, __BitLen) ) \
	)

#define SET_BITS_TO_LE_1BYTE(__pStart, __BitOffset, __BitLen, __Value) \
{ \
	*((pu1Byte)(__pStart)) = \
		EF1Byte( \
			LE_BITS_CLEARED_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
			| \
			( (((u1Byte)__Value) & BIT_LEN_MASK_8(__BitLen)) << (__BitOffset) ) \
		); \
}


#define SET_80211_HDR_DURATION(_hdr, _val)	\
	WriteEF2Byte((pu1Byte)(_hdr)+FRAME_OFFSET_DURATION, _val)

#define SET_80211_HDR_ORDER(_hdr, _val)					SET_BITS_TO_LE_2BYTE(_hdr, 15, 1, _val)

#define SET_80211_HDR_FRAME_CONTROL(_hdr, _val)				WriteEF2Byte(_hdr, _val)
#define SET_80211_HDR_TYPE_AND_SUBTYPE(_hdr, _val)			WriteEF1Byte(_hdr, _val)

//------------------------------------------------------------
// The HT Control field
//------------------------------------------------------------
#define SET_HT_CTRL_CSI_STEERING(_pEleStart, _val)					SET_BITS_TO_LE_1BYTE((_pEleStart)+2, 6, 2, _val)
#define SET_HT_CTRL_NDP_ANNOUNCEMENT(_pEleStart, _val)				SET_BITS_TO_LE_1BYTE((_pEleStart)+3, 0, 1, _val)
#define GET_HT_CTRL_NDP_ANNOUNCEMENT(_pEleStart)					LE_BITS_TO_1BYTE((_pEleStart)+3, 0, 1)

#define sMacHdrLng					24
#define sHTCLng	4


//
// TX report 2 format in Rx desc
//
#define GET_TX_RPT2_DESC_PKT_LEN_88E(__pRxStatusDesc)				LE_BITS_TO_4BYTE( __pRxStatusDesc, 0, 9)
#define GET_TX_RPT2_DESC_MACID_VALID_1_88E(__pRxStatusDesc) 	LE_BITS_TO_4BYTE( __pRxStatusDesc+16, 0, 32)
#define GET_TX_RPT2_DESC_MACID_VALID_2_88E(__pRxStatusDesc) 	LE_BITS_TO_4BYTE( __pRxStatusDesc+20, 0, 32)

#define GET_TX_REPORT_TYPE1_RERTY_0(__pAddr)						LE_BITS_TO_4BYTE( __pAddr, 0, 16)
#define GET_TX_REPORT_TYPE1_RERTY_1(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+2, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_2(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+3, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_3(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+4, 0, 8)
#define GET_TX_REPORT_TYPE1_RERTY_4(__pAddr)						LE_BITS_TO_1BYTE( __pAddr+4+1, 0, 8)
#define GET_TX_REPORT_TYPE1_DROP_0(__pAddr) 					LE_BITS_TO_1BYTE( __pAddr+4+2, 0, 8)
#define GET_TX_REPORT_TYPE1_DROP_1(__pAddr) 					LE_BITS_TO_1BYTE( __pAddr+4+3, 0, 8)

#define GET_RX_STATUS_DESC_RPT_SEL_8812(__pRxStatusDesc)			LE_BITS_TO_4BYTE( __pRxStatusDesc+8, 28, 1)
typedef enum _WIRELESS_MODE {
	WIRELESS_MODE_UNKNOWN = 0x00,
	WIRELESS_MODE_A = 0x01,
	WIRELESS_MODE_B = 0x02,
	WIRELESS_MODE_G = 0x04,
	WIRELESS_MODE_AUTO = 0x08,
	WIRELESS_MODE_N_24G = 0x10,
	WIRELESS_MODE_N_5G = 0x20,
	WIRELESS_MODE_AC_5G = 0x40,
	WIRELESS_MODE_AC_24G = 0x80	
} WIRELESS_MODE;
#endif
