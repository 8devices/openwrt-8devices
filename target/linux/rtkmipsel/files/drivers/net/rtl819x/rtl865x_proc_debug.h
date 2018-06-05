/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.                                                    
* 
* Program : just for driver debug
* Abstract :                                                           
* Author : Hyking Liu (Hyking_liu@realsil.com.tw)               
* -------------------------------------------------------
*/
#ifndef RTL865X_PROC_DEBUG_H
#define RTL865X_PROC_DEBUG_H
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define RTL865X_PROC_DIR_NAME "rtl865x"

int32 rtl865x_proc_debug_init(void);
int32 rtl865x_proc_debug_cleanup(void);
#endif

