/*
 *  D-MEM supporting module for RTL8190 802.11N wireless NIC on RTL865x platform
 *
 *  $Id: 8192cd_dmem.c,v 1.2 2010/01/19 06:04:03 jimmylin Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#ifndef _8192CD_DMEM_C_
#define _8192CD_DMEM_C_

#include "./8192cd.h"
//#include "./8190n_fastExtDev.h"
#include "./8192cd_cfg.h"

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"

#define RTL8192CD_MAX_SPEEDUP_STA			2
#define RTL8192CD_SPEEDUP_PRIV_COUNT		1

/* ========================== All variables using D-MEM ========================== */
static void rtl8192cd_dmem_AID_OBJ_init(void);
static void *rtl8192cd_dmem_AID_OBJ_alloc(void *miscInfo);
static void rtl8192cd_dmem_AID_OBJ_free(void *miscInfo);

static void rtl8192cd_dmem_pmib_init(void);
static void *rtl8192cd_dmem_pmib_alloc(void *miscInfo);
static void rtl8192cd_dmem_pmib_free(void *miscInfo);

#ifdef PRIV_STA_BUF
	extern struct aid_obj *alloc_sta_obj(struct rtl8192cd_priv*);
	extern void free_sta_obj(struct rtl8192cd_priv *priv, struct aid_obj *obj);
#endif

static _rtl8192cd_dmem_callBack_t _8192cd_dmem_callBack_list[] =
{
		/*		ID			Init CallBack				Allocate CallBack				Free CallBack */
		{		AID_OBJ,	rtl8192cd_dmem_AID_OBJ_init,	rtl8192cd_dmem_AID_OBJ_alloc,		rtl8192cd_dmem_AID_OBJ_free},
		{		PMIB,		rtl8192cd_dmem_pmib_init,		rtl8192cd_dmem_pmib_alloc,		rtl8192cd_dmem_pmib_free},
		/* ==================================================================== */
		{		_RTL8192CD_DMEM_ITEM_MAX,	NULL,								NULL},
};

/* ========================== External APIs of D-MEM module ========================== */

/*
	Initiation function for DMEM library
*/
void rtl8192cd_dmem_init( void )
{
	_rtl8192cd_dmem_callBack_t *ptr;

	ptr = &_8192cd_dmem_callBack_list[0];

	while (	(ptr->id > _RTL8192CD_DMEM_ITEM_MIN) &&
			(ptr->id < _RTL8192CD_DMEM_ITEM_MAX))
	{
		/* Call the Callback function to decide the memory of allocated */
		if (ptr->initCallBackFunc)
		{
			((_dummyFunc_void_void)(ptr->initCallBackFunc))();
		}

		/* Next Entry */
		ptr ++;
	}
}

void *rtl8192cd_dmem_alloc( enum _RTL8192CD_DMEM_ITEM_ID id, void *miscInfo )
{
	void *retval;
	_rtl8192cd_dmem_callBack_t *ptr;

	retval = NULL;

	if (	(id <= _RTL8192CD_DMEM_ITEM_MIN) ||
		(id >= _RTL8192CD_DMEM_ITEM_MAX))
	{
		printk("%s %d : ERROR (%d)\n", __FUNCTION__, __LINE__, id);
		goto out;
	}

	ptr = &_8192cd_dmem_callBack_list[0];

	while ( ptr->allcateCallBackFunc )
	{
		if ( ptr->id == id )
		{
			/* Call the Callback function to decide the memory of allocated */
			retval = ((_dummyFunc_voidStar_voidStar)(ptr->allcateCallBackFunc))(miscInfo);
			goto out;
		}
		/* Next Entry */
		ptr ++;
	}

out:
	return retval;
}

void rtl8192cd_dmem_free( enum _RTL8192CD_DMEM_ITEM_ID id, void *miscInfo )
{
	_rtl8192cd_dmem_callBack_t *ptr;

	if (	(id <= _RTL8192CD_DMEM_ITEM_MIN) ||
		(id >= _RTL8192CD_DMEM_ITEM_MAX))
	{
		printk("%s %d : ERROR (%d)\n", __FUNCTION__, __LINE__, id);
		goto out;
	}

	ptr = &_8192cd_dmem_callBack_list[0];

	while ( ptr->freeCallBackFunc )
	{
		if ( ptr->id == id )
		{
			/* Call the Callback function to decide the memory of allocated */
			((_dummyFunc_void_voidStar)(ptr->freeCallBackFunc))(miscInfo);
			goto out;
		}
		/* Next Entry */
		ptr ++;
	}

out:
	return;
}


/* ========================== Internal APIs for per-variable of D-MEM module ========================== */

/* ==============================================
  *
  *		AID_OBJ
  *
  *
  * ============================================== */
__DRAM_IN_865X struct aid_obj _rtl8192cd_aid_Array[RTL8192CD_MAX_SPEEDUP_STA];
void *_rtl8192cd_aid_externalMem_Array[NUM_STAT];

static void rtl8192cd_dmem_AID_OBJ_init(void)
{
	memset(_rtl8192cd_aid_Array, 0, sizeof(struct aid_obj) * RTL8192CD_MAX_SPEEDUP_STA);
	memset(_rtl8192cd_aid_externalMem_Array, 0, sizeof(_rtl8192cd_aid_externalMem_Array));
}

static void *rtl8192cd_dmem_AID_OBJ_alloc(void *miscInfo)
{
	/* For AID_OBJ : miscInfo would be [unsigned int *] to decision the index of aidarray to allocate */
	unsigned int index = *((unsigned int*)miscInfo);

	if (	(index < 0) ||
		(index >= NUM_STAT))
	{
		printk("%s %d : ERROR ( Index : %d )\n", __FUNCTION__, __LINE__, index);
		return NULL;
	}

	/* Allocate from external memory */
	if ( index >= RTL8192CD_MAX_SPEEDUP_STA )
	{
#ifdef PRIV_STA_BUF
		_rtl8192cd_aid_externalMem_Array[index] = alloc_sta_obj(NULL);
#else
		_rtl8192cd_aid_externalMem_Array[index] = kmalloc(sizeof(struct aid_obj), GFP_ATOMIC);
#endif
		if (_rtl8192cd_aid_externalMem_Array[index] == NULL)
		{
			printk("%s %d : Error : Allocation FAILED!\n", __FUNCTION__, __LINE__);
			return NULL;
		}
		return _rtl8192cd_aid_externalMem_Array[index];
	}

	memset(&(_rtl8192cd_aid_Array[index]), 0, sizeof(struct aid_obj));

	return (void*)(&(_rtl8192cd_aid_Array[index]));
}

static void rtl8192cd_dmem_AID_OBJ_free(void *miscInfo)
{
	/* For AID_OBJ : miscInfo would be [unsigned int *] to decision the index of aidarray to free */
	unsigned int index = *((unsigned int*)miscInfo);

	if (	(index < 0) ||
		(index >= NUM_STAT))
	{
		printk("%s %d : ERROR ( Index : %d )\n", __FUNCTION__, __LINE__, index);
		return;
	}

	/* Free memory to external memory module */
	if ( index >= RTL8192CD_MAX_SPEEDUP_STA )
	{
		if ( _rtl8192cd_aid_externalMem_Array[index] )
		{
#ifdef PRIV_STA_BUF
			free_sta_obj(NULL, _rtl8192cd_aid_externalMem_Array[index]);
#else
			kfree(_rtl8192cd_aid_externalMem_Array[index]);
#endif
			_rtl8192cd_aid_externalMem_Array[index] = NULL;
		}

		return;
	}

	memset(&(_rtl8192cd_aid_Array[index]), 0, sizeof(struct aid_obj));
}


/* =================== The following variable are mapped to PRIV =================== */

/* ==============================================
  *
  *		PMIB
  *
  *
  * ============================================== */
__DRAM_IN_865X struct wifi_mib _rtl8192cd_pmib[RTL8192CD_SPEEDUP_PRIV_COUNT];
int _rtl8192cd_pmib_usageMap[RTL8192CD_SPEEDUP_PRIV_COUNT];

static void rtl8192cd_dmem_pmib_init(void)
{
	memset(_rtl8192cd_pmib_usageMap, 0, sizeof(int) * RTL8192CD_SPEEDUP_PRIV_COUNT);
	memset(_rtl8192cd_pmib, 0, sizeof(struct wifi_mib) * RTL8192CD_SPEEDUP_PRIV_COUNT);
}

static void *rtl8192cd_dmem_pmib_alloc(void *miscInfo)
{
	int idx ;

	/* miscInfo is useless */
	for ( idx = 0 ; idx < RTL8192CD_SPEEDUP_PRIV_COUNT ; idx ++ )
	{
		if ( _rtl8192cd_pmib_usageMap[idx] == 0 )
		{	/* Unused entry : use it */
			_rtl8192cd_pmib_usageMap[idx] = 1;
			memset(&(_rtl8192cd_pmib[idx]), 0, sizeof(struct wifi_mib));
			return &(_rtl8192cd_pmib[idx]);
		}
	}

	/* Allocate from externel memory if speedup PMIB is exhausted */
	return kmalloc(sizeof(struct wifi_mib), GFP_ATOMIC);

}

static void rtl8192cd_dmem_pmib_free(void *miscInfo)
{
	int idx;

	/* miscInfo is pointed to the address of PMIB to free */

	/* Free PMIB if it is speeded up by DMEM */
	for ( idx = 0 ; idx < RTL8192CD_SPEEDUP_PRIV_COUNT ; idx ++ )
	{
		if ( (unsigned long)(&(_rtl8192cd_pmib[idx])) == (unsigned long)miscInfo )
		{	/* Entry is found : free it */
			memset(&(_rtl8192cd_pmib[idx]), 0, sizeof(struct wifi_mib));
			_rtl8192cd_pmib_usageMap[idx] = 0;
			return;
		}
	}

	/* It would be allocated from external memory: kfree it */
	kfree(miscInfo);

}
#endif // RTL8192CD_VARIABLE_USED_DMEM

#endif

