/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  IGMP glue file
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/


#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include "AsicDriver/rtl865xc_asicregs.h"

#ifdef __linux__
#include <linux/mm.h>
#include	<linux/gfp.h>
#endif

/*
@func int32	| rtl_glueMutexLock	|  glue function for system mutex lock
@rvalue 0| always return 0;
@comm 
 user should modify this function to glue different OS.
*/

void rtl_glueMutexLock(void)
{
	//rtlglue_drvMutexLock();
	return;
}



/*
@func int32	| rtl_glueMutexUnlock	|  Glue function for system mutex unlock 
@rvalue 0| always return 0;
@comm 
 User should modify this function to glue different OS.
*/
void rtl_glueMutexUnlock(void)
{
	//rtlglue_drvMutexUnlock();
	return;
}


/*
@func void*	| rtl_glueMalloc	|   Glue function for memory allocation
@parm uint32	| NBYTES		| Specifies the number of memory bytes to be allocated
@rvalue void*| The memory pointer after allocation
@comm 
 User should modify this function according to different OS.
*/
void *rtl_glueMalloc(uint32 NBYTES)
{
#ifndef RTL_MULTICAST_SNOOPING_TEST
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
		
#else
	return malloc(NBYTES);
	
#endif
	
}

/*
@func void	| rtl_glueFree	|Glue function for memory free
@parm void *	| APTR		| Specifies the memory buffer to be freed
@comm 
  User should modify this function according to different OS.
*/
void rtl_glueFree(void *APTR)
{

#ifndef RTL_MULTICAST_SNOOPING_TEST
	kfree(APTR);	
#else
	free(APTR);
#endif

}






