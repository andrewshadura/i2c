#include "i2c.h"

#define GPIO_VGA_DDC            0x0060
#define GPIO_DVI_DDC            0x0064
#define GPIO_MONID              0x0068
#define GPIO_CRT2_DDC           0x006c

/* GPIO bit constants */
#define GPIO_A_0				(1 <<  0)
#define GPIO_A_1				(1 <<  1)
#define GPIO_Y_0				(1 <<  8)
#define GPIO_Y_1				(1 <<  9)
#define GPIO_EN_0				(1 << 16)
#define GPIO_EN_1				(1 << 17)
#define GPIO_MASK_0				(1 << 24)
#define GPIO_MASK_1				(1 << 25)
#define VGA_DDC_DATA_OUTPUT		GPIO_A_0
#define VGA_DDC_CLK_OUTPUT		GPIO_A_1
#define VGA_DDC_DATA_INPUT		GPIO_Y_0
#define VGA_DDC_CLK_INPUT		GPIO_Y_1
#define VGA_DDC_DATA_OUT_EN		GPIO_EN_0
#define VGA_DDC_CLK_OUT_EN		GPIO_EN_1

#define OUTREG(reg, state) (*((PUINT32)(reg)) = state)
#define INREG(reg) (*((PUINT32)(reg)))

// mmio??

static VOID radeon_setsda(struct i2c_driver_t *st, UINT32 state)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	val = INREG(st->ddc_base) & ~(VGA_DDC_DATA_OUT_EN);
	if (!state)
		val |= VGA_DDC_DATA_OUT_EN;

	OUTREG(st->ddc_base, val);
	(void)INREG(st->ddc_base);
}

static VOID radeon_setscl(struct i2c_driver_t *st, UINT32 state)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	val = INREG(st->ddc_base) & ~(VGA_DDC_CLK_OUT_EN);
	if (!state)
		val |= VGA_DDC_CLK_OUT_EN;

	OUTREG(st->ddc_base, val);
	(void)INREG(st->ddc_base);
}

static UINT32 radeon_getsda(struct i2c_driver_t *st)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	val = INREG(st->ddc_base);

	return (val & VGA_DDC_DATA_INPUT) ? 1 : 0;
}

static UINT32 radeon_getscl(struct i2c_driver_t *st)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	val = INREG(st->ddc_base);

	return (val & VGA_DDC_CLK_INPUT) ? 1 : 0;
}

static VOID radeon_init(struct i2c_driver_t *st)
{
	st->ddc_base = GPIO_CRT2_DDC;

	st->setsda = radeon_setsda;
	st->setscl = radeon_setscl;
	st->getsda = radeon_getsda;
	st->getscl = radeon_getscl;
	st->udelay = 10;

	/* Raise SCL and SDA */
	st->setsda(st, 1);
	st->setscl(st, 1);
	Sleep(20);
}
