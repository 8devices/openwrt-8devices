/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author : Louis Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: gpio.h,v 1.1 2009/03/23 11:54:49 jasonwang Exp $
*/

#ifndef __GPIO_H__
#define __GPIO_H__

#define PECNR	 (PDEPTCR+1)
#define PEDIR (PDEDIR+1)
#define PEDATA (PDEDAT+1)

/*
			9001 	8651  dir
----------------------------------
	RESET			D5		O    (slic2)
	RESET	A0		E7		O    (slic1)
	DI		B2		E6		I
	DO		B1		E5		O
	CLK		B3		E4		O
	CS		B0		E3		O
	INT		A4		E2		I    (slic2)
	INT				E1		I    (slic1)
	RELAY			E0		O		

//CS is active low
#define si3120_RS	0x80
#define si3210_INT	0x04
#define si3210_DI	0x40
#define si3210_DO	0x20
#define si3210_CLK	0x10
#define si3210_CS	0x08
*/

/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
};

/* define GPIO dedicate peripheral pin */
enum GPIO_PERIPHERAL
{
	GPIO_PERI_GPIO = 0,
	GPIO_PERI_TYPE0 = 0x2,
	GPIO_PERI_TYPE1 = 0x3,
};


/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_FALLING_EDGE,
	GPIO_INT_RISING_EDGE,
	GPIO_INT_BOTH_EDGE,
};

typedef uint32 gpioID;


/*
 * Every pin of GPIO port can be mapped to a unique ID.
 * All the access to a GPIO pin must use the ID.
 * This macro is used to map the port and pin into the ID.
 */
#define GPIO_ID(port,pin) ((uint32)port<<8|(uint32)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>8)
#define GPIO_PIN(id) (id&0xff)



int32 _rtl865x_initGpioPin(gpioID gpioId, 
				enum GPIO_PERIPHERAL dedicate, 
				enum GPIO_DIRECTION direction, 
				enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl865x_getGpioDataBit( uint32 gpioId, uint32* data );
int32 _rtl865x_setGpioDataBit( uint32 gpioId, uint32 data );

#endif/*__GPIO__*/

