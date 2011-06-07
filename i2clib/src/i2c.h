#ifndef __I2C_H___
#define __I2C_H___

#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#define LOW               0
#define HIGH              1

struct i2c_driver_t
{
	UINT32 ddc_base;
	UCHAR *mmio_start_virtual;
	VOID (*setsda) (struct i2c_struct *st, UINT32 state);
	VOID (*setscl) (struct i2c_struct *st, UINT32 state);
	UINT32 (*getsda) (struct i2c_struct *st);
	UINT32 (*getscl) (struct i2c_struct *st);
	UINT32 udelay;		/*	half clock cycle time in us,
							minimum 2 us for fast-mode I2C,
							minimum 5 us for standard-mode I2C and SMBus,
							maximum 50 us for SMBus */
};

#endif /* __I2C_H___ */
