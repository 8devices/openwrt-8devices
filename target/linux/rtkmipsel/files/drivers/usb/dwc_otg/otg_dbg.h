#ifndef	_OTG_DBG__H_
#define	_OTG_DBG__H_		1

#if 0
#define bh_otg_dbg(format, arg...)		\
	printk(format , ## arg)
#else
#define bh_otg_dbg(format, arg...)
#endif

#if 0
#define bh_otg_dbg2(format, arg...)		\
	printk(format , ## arg)
#else
#define bh_otg_dbg2(format, arg...)
#endif

#if 1
#define bh_otg_dwc(format, arg...)		\
	printk(format , ## arg)
#else
#define bh_otg_dwc(format, arg...)
#endif

#define bh_otg_info(format, arg...)		\
	printk(format , ## arg)

#endif /* _OTG_DBG__H_ */
