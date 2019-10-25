/*
 *  Header of D-MEM supporting module for RTL8190 802.11N wireless NIC on RTL865x platform
 *
 *  $Id: 8192cd_dmem.h,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_DMEM_H
#define _8192CD_DMEM_H

/* ========================= External ========================= */

enum _RTL8192CD_DMEM_ITEM_ID {
	_RTL8192CD_DMEM_ITEM_MIN,
	/* ============================== Add here ============================== */
	AID_OBJ,
	PMIB,
	/* =================================================================== */
	_RTL8192CD_DMEM_ITEM_MAX,
};

void rtl8192cd_dmem_init( void );
void *rtl8192cd_dmem_alloc( enum _RTL8192CD_DMEM_ITEM_ID id, void *miscInfo );
void rtl8192cd_dmem_free( enum _RTL8192CD_DMEM_ITEM_ID id, void *miscInfo );


/* ========================= Internal ========================= */
typedef void* (*_dummyFunc_voidStar_voidStar)(void*);
typedef void (*_dummyFunc_void_voidStar)(void*);
typedef void (*_dummyFunc_void_void)(void);
typedef struct _rtl8192cd_dmem_list_s
{
	int		id;
	void *	initCallBackFunc;
	void *	allcateCallBackFunc;
	void *	freeCallBackFunc;
} _rtl8192cd_dmem_callBack_t;

#endif /* _8192CD_DMEM_H */
