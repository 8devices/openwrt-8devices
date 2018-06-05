
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "../AsicDriver/asicRegs.h"
#include "gpio.h"

#define RTL_GPIO_PABDIR         PABCD_DIR
#define RTL_GPIO_PABDATA        PABCD_DAT
#define RTL_GPIO_PABCNR         PABCD_CNR

/*
 * 0 -- less debug information
 * 5 -- more debug information
 */
#define _GPIO_DEBUG_ 0

enum GPIO_FUNC
{
	GPIO_FUNC_DEDICATE,
	GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE,
	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};
#define PABCCNR PABCD_CNR  
#define PDEPTCR PEFGH_CNR 
#define PFGHICNR PEFGH_CNR 

static uint32 regGpioControl[] = 
{
	PABCD_CNR, /* Port A */
	PABCD_CNR, /* Port B */
	PABCD_CNR, /* Port C */
	PABCD_CNR, /* Port D */
	PEFGH_CNR, /* Port E */
	PEFGH_CNR,/* Port F */
	PEFGH_CNR,/* Port G */
	PEFGH_CNR,/* Port H */
};

static uint32 bitStartGpioControl[] =
{
	0, /* Port A */
	8, /* Port B */
	16,  /* Port C */
	24, /* Port D */
	0, /* Port E */
	8, /* Port F */
	16, /* Port G */
	24,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioDedicatePeripheralType[] = 
{
	PABCD_PTYPE, /* Port A */
	PABCD_PTYPE, /* Port B */
	PABCD_PTYPE, /* Port C */
	PABCD_PTYPE,  /* Port D */
	PEFGH_PTYPE,  /* Port E */
	PEFGH_PTYPE,        /* Port F */
	PEFGH_PTYPE,        /* Port G */
	PEFGH_PTYPE,        /* Port H */
	0,        /* Port I */
};

static uint32 bitStartGpioDedicatePeripheralType[] =
{
	0, /* Port A */
	8, /* Port B */
	16,  /* Port C */
	24, /* Port D */
	0, /* Port E */
	8,  /* Port F */
	16,  /* Port G */
	24,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioDirection[] =
{
	PABCD_DIR, /* Port A */
	PABCD_DIR, /* Port B */
	PABCD_DIR, /* Port C */
	PABCD_DIR,  /* Port D */
	PEFGH_DIR,  /* Port E */
	PEFGH_DIR,/* Port F */
	PEFGH_DIR,/* Port G */
	PEFGH_DIR,/* Port H */
	0,/* Port I */
};

static uint32 bitStartGpioDirection[] =
{
	0, /* Port A */
	8, /* Port B */
	16,  /* Port C */
	24, /* Port D */
	0, /* Port E */
	8, /* Port F */
	16, /* Port G */
	24,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioData[] =
{
	PABCD_DAT, /* Port A */
	PABCD_DAT, /* Port B */
	PABCD_DAT, /* Port C */
	PABCD_DAT,  /* Port D */
	PEFGH_DAT,  /* Port E */
	PEFGH_DAT,/* Port F */
	PEFGH_DAT,/* Port G */
	PEFGH_DAT,/* Port H */
	0,/* Port I */
};

static uint32 bitStartGpioData[] =
{
	0, /* Port A */
	8, /* Port B */
	16,  /* Port C */
	24, /* Port D */
	0, /* Port E */
	8, /* Port F */
	16, /* Port G */
	24,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioInterruptStatus[] =
{
	PABCD_ISR, /* Port A */
	PABCD_ISR, /* Port B */
	PABCD_ISR, /* Port C */
	PABCD_ISR,  /* Port D */
	PEFGH_ISR,  /* Port E */
	PEFGH_ISR,/* Port F */
	PEFGH_ISR,/* Port G */
	PEFGH_ISR,/* Port H */
	0,/* Port I */
};

static uint32 bitStartGpioInterruptStatus[] =
{
	0, /* Port A */
	8, /* Port B */
	16,  /* Port C */
	24, /* Port D */
	0, /* Port E */
	8, /* Port F */
	16, /* Port G */
	24,  /* Port H */
	0,  /* Port I */
};

static uint32 regGpioInterruptEnable[] =
{
	PAB_IMR,/* Port A */
	PAB_IMR,/* Port B */
	PCD_IMR, /* Port C */
	PCD_IMR,/* Port D */
	PEF_IMR,/* Port E */
	PEF_IMR,/* Port F */
	PGH_IMR,/* Port G */
	PGH_IMR,/* Port H */
	0,/* Port I */
};

static uint32 bitStartGpioInterruptEnable[] =
{
	0,  /* Port A */
	16, /* Port B */
	0,  /* Port C */
	16, /* Port D */
	0,  /* Port E */
	16, /* Port F */
	0,  /* Port G */
	16, /* Port H */
};

#undef rtlglue_printf
#ifdef CONFIG_PRINTK
	#define rtlglue_printf	printk
#else
	#define rtlglue_printf	panic_printk
#endif

/*
@func int32 | _getGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@rvalue uint32 | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
{

	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
			if ( REG32(regGpioControl[port]) & ( (uint32)1 << (pin+bitStartGpioControl[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE:

			if ( REG32(regGpioDedicatePeripheralType[port]) & ( (uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DIRECTION:
			if ( REG32(regGpioDirection[port]) & ( (uint32)1 << (pin+bitStartGpioDirection[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
			if ( REG32(regGpioData[port]) & ( (uint32)1 << (pin+bitStartGpioData[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
			return ( REG32(regGpioInterruptEnable[port]) >> (pin*2+bitStartGpioInterruptEnable[port]) ) & (uint32)0x3;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
			if ( REG32(regGpioInterruptStatus[port]) & ( (uint32)1 << (pin+bitStartGpioInterruptStatus[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_MAX:
			break;
	}
	return 0xffffffff;
}


/*
@func int32 | _setGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm uint32 | pin | pin number
@parm uint32 | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
{
	
#if _GPIO_DEBUG_ >= 4
	rtlglue_printf("[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
#endif

	switch( func )
	{
		case GPIO_FUNC_DEDICATE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioControl[port]=0x%08x  bitStartGpioControl[port]=%d\n", __FUNCTION__, __LINE__, regGpioControl[port], bitStartGpioControl[port] );
#endif
			if ( data )
				REG32(regGpioControl[port]) |= (uint32)1 << (pin+bitStartGpioControl[port]);
			else
				REG32(regGpioControl[port]) &= ~((uint32)1 << (pin+bitStartGpioControl[port]));
			break;
			
		case GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDedicatePeripheralType[port]=0x%08x  bitStartGpioDedicatePeripheralType[port]=%d\n", __FUNCTION__, __LINE__, regGpioDedicatePeripheralType[port], bitStartGpioDedicatePeripheralType[port] );
#endif
			if ( data )
				REG32(regGpioDedicatePeripheralType[port]) |= (uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]);
			else
				REG32(regGpioDedicatePeripheralType[port]) &= ~((uint32)1 << (pin+bitStartGpioDedicatePeripheralType[port]));
			break;
			
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioDirection[port]=0x%08x  bitStartGpioDirection[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirection[port], bitStartGpioDirection[port] );
#endif
			if ( data )
				REG32(regGpioDirection[port]) |= (uint32)1 << (pin+bitStartGpioDirection[port]);
			else
				REG32(regGpioDirection[port]) &= ~((uint32)1 << (pin+bitStartGpioDirection[port]));
			break;

		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioData[port]=0x%08x  bitStartGpioData[port]=%d\n", __FUNCTION__, __LINE__, regGpioData[port], bitStartGpioData[port] );
#endif
			if ( data )
				REG32(regGpioData[port]) |= (uint32)1 << (pin+bitStartGpioData[port]);
			else
				REG32(regGpioData[port]) &= ~((uint32)1 << (pin+bitStartGpioData[port]));
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptEnable[port]=0x%08x  bitStartGpioInterruptEnable[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnable[port], bitStartGpioInterruptEnable[port] );
#endif
			REG32(regGpioInterruptEnable[port]) &= ~((uint32)0x3 << (pin*2+bitStartGpioInterruptEnable[port]));
			REG32(regGpioInterruptEnable[port]) |= (uint32)data << (pin*2+bitStartGpioInterruptEnable[port]);
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			rtlglue_printf("[%s():%d] regGpioInterruptStatus[port]=0x%08x  bitStartGpioInterruptStatus[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatus[port], bitStartGpioInterruptStatus[port] );
#endif
			if ( data )
				REG32(regGpioInterruptStatus[port]) |= (uint32)1 << (pin+bitStartGpioInterruptStatus[port]);
			else
				REG32(regGpioInterruptStatus[port]) &= ~((uint32)1 << (pin+bitStartGpioInterruptStatus[port]));
			break;

		case GPIO_FUNC_MAX:
			break;
	}
}


/*
@func int32 | _rtl865x_initGpioPin | Initiate a specifed GPIO port.
@parm uint32 | gpioId | The GPIO port that will be configured
@parm enum GPIO_PERIPHERAL | dedicate | Dedicated peripheral type
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
int32 _rtl865x_initGpioPin( gpioID gpioId, 
	enum GPIO_PERIPHERAL dedicate, 
	enum GPIO_DIRECTION direction, 
	enum GPIO_INTERRUPT_TYPE interruptEnable )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;

	switch( dedicate )
	{
		case GPIO_PERI_GPIO:
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 0 );

			//if ( port == GPIO_PORT_D || port == GPIO_PORT_E ) /*change Mii interface IO as GPIO pins*/
			//	REG32( MISCCR ) |= P5_LINK_PCMCIA << P5_LINK_OFFSET;

			break;
		case GPIO_PERI_TYPE0:
			if( port > GPIO_PORT_E ) return FAILED;
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 1 );
			_setGpio( GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE, port, pin, 0 );
			break;
		case GPIO_PERI_TYPE1:
			if( port > GPIO_PORT_E ) return FAILED;
			_setGpio( GPIO_FUNC_DEDICATE, port, pin, 1 );
			_setGpio( GPIO_FUNC_DEDICATE_PERIPHERAL_TYPE, port, pin, 1 );
			break;
	}
	
	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );

	return SUCCESS;
}


/*
@func int32 | _rtl865x_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_getGpioDataBit( uint32 gpioId, uint32* pData )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
#if _GPIO_DEBUG_ >= 3
	rtlglue_printf("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
#endif

	return SUCCESS;
}


/*
@func int32 | _rtl865x_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm uint32 | gpioId | GPIO ID
@parm uint32 | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_setGpioDataBit( uint32 gpioId, uint32 data )
{
	uint32 port = GPIO_PORT( gpioId );
	uint32 pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	if ( pin >= 8 ) return FAILED;
#if 0
	if ( _getGpio( GPIO_FUNC_DIRECTION, port, pin ) == GPIO_DIR_IN ) return FAILED; /* read only */
#endif

#if _GPIO_DEBUG_ >= 3
	rtlglue_printf("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, data );
#endif
	_setGpio( GPIO_FUNC_DATA, port, pin, data );

	return SUCCESS;
}

