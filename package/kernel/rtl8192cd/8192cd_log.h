
#ifndef	8192CD_LOG_H
#define	8192CD_LOG_H

#if defined(CONFIG_RTL_LOG_DEBUG)

	#if defined(LOG_ERROR)
	#undef LOG_ERROR
	#define LOG_ERROR(fmt, args...) do{ \
		if(RTL_LogTypeMask.ERROR&&RTL_LogModuleMask.WIRELESS&&LOG_LIMIT)scrlog_printk("WLS-ERROR:"fmt, ## args); \
			}while(0)
	#endif

	#if defined(LOG_MEM_ERROR)
	#undef LOG_MEM_ERROR
	#define LOG_MEM_ERROR(fmt, args...) do{ \
		if(RTL_LogTypeMask.ERROR&&RTL_LogErrorMask.MEM&&RTL_LogModuleMask.WIRELESS&&LOG_LIMIT)scrlog_printk("WLS-MEM-ERROR:"fmt, ## args); \
			}while(0)
	#endif

	#if defined(LOG_SKB_ERROR)
	#undef LOG_SKB_ERROR
	#define LOG_SKB_ERROR(fmt, args...) do{ \
		if(RTL_LogTypeMask.ERROR&&RTL_LogErrorMask.SKB&&RTL_LogModuleMask.WIRELESS&&LOG_LIMIT)scrlog_printk("WLS-SKB-ERROR:"fmt, ## args); \
			}while(0)
	#endif
	
	#if defined(LOG_WARN)		
	#undef LOG_WARN	
	#define LOG_WARN(fmt, args...) do{ \
		if(RTL_LogTypeMask.WARN&&RTL_LogModuleMask.WIRELESS&&LOG_LIMIT)scrlog_printk("WLS-WARN:"fmt, ## args); \
			}while(0)
	#endif
	
	#if defined(LOG_INFO)		
	#undef LOG_INFO	
	#define LOG_INFO(fmt, args...) do{ \
		if(RTL_LogTypeMask.INFO&&RTL_LogModuleMask.WIRELESS&&LOG_LIMIT)scrlog_printk("WLS-INFO:"fmt, ## args); \
			}while(0)
	#endif
	
#endif

#endif

