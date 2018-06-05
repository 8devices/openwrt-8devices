#ifndef __RTKN_CHIP_PARAM_H__
#define __RTKN_CHIP_PARAM_H__

/* temp code */
#define HY27UT084G2M	0xADDC8425	//MLC, 512 MB, 1 dies, BB check at last page, SLOW nand

/* winbond */
#define W29N01GV			0xEFF18095

/* Micorn */
#define MT29F64G08CBABA	 	0x2C64444B	// Micron 64G	(MLC)
#define MT29F64G08CBABB	 	0x2C64444B	// Micron 64G	(MLC)
#define MT29F32G08CBADA	 	0x2C44444B	// Micron 32Gb (MLC)

/* Toshiba */
#define TC58TEG6DCJT		0x98DE8493	// Toshiba 64Gb (MLC)
#define TC58TEG5DCJT		0x98D78493	// Toshiba 32Gb (MLC)
//#define TC58TEG6DDK			0x98DE9493	0x9394de98

/* MXIC */
#define MX30LF2G18AC		 0xC2da9095

static nand_chip_param_T nand_chip_id[] =
{
	{W29N01GV,			0,	0xff, 	0x01, 0x01, 0x01, 0x00,	0,	0,	0,	0,	NULL,NULL,NULL},
	{MT29F64G08CBABA,	0,	0x28,	0x01, 0x01, 0x01, 0x28,	1,	1,	0,	7,	NULL,NULL,NULL},
	{MT29F32G08CBADA,	0,	0x28,	0x00, 0x00, 0x00, 0x28,	1,	1,	0,	7,	NULL,NULL,NULL},
	/*	Toshiba */
	{TC58TEG5DCJT,		0,	0x28, 	0x01, 0x01, 0x01, 0x28,	1,	1,	0,	6,	NULL,NULL,NULL},
	{TC58TEG6DCJT,		0,	0x28, 	0x01, 0x01, 0x01, 0x28,	1,	1,	0,	6,	NULL,NULL,NULL},
	//{TC58TEG6DDK,		0,	0x28, 	0x01, 0x01, 0x01, 0x28,	1,	1,	0,	7,	NULL},
	/* MXIC */
	{MX30LF2G18AC,		0,	0xff, 	0x01, 0x01, 0x01, 0x00,	0,	0,	0,	0,	NULL,NULL,NULL}
};

#endif