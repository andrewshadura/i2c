#include "i2c.h"

#define GPIOA           0x5010
#define GPIOB           0x5014
#define GPIOC           0x5018 /* this may be external DDC on i830 */
#define GPIOD           0x501C /* this is DVO DDC */
#define GPIOE           0x5020 /* this is DVO i2C */
#define GPIOF           0x5024
#define GPIOG		    0x5028
#define GPIOH			0x502c

/* bit locations in the registers */
#define SCL_DIR_MASK	0x0001
#define SCL_DIR			0x0002
#define SCL_VAL_MASK	0x0004
#define SCL_VAL_OUT		0x0008
#define SCL_VAL_IN		0x0010
#define SDA_DIR_MASK	0x0100
#define SDA_DIR			0x0200
#define SDA_VAL_MASK	0x0400
#define SDA_VAL_OUT		0x0800
#define SDA_VAL_IN		0x1000

#define OUTREG(reg, state) (*((PUINT32)(reg)) = state)
#define INREG(reg) (*((PUINT32)(reg)))

static VOID i810_setsda(struct i2c_struct *st, UINT32 state)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	OUTREG(mmio + st->ddc_base, (state ? SDA_VAL_OUT : 0) |
		SDA_DIR | SDA_DIR_MASK | SDA_VAL_MASK);
	val = INREG(mmio + st->ddc_base);
}

static VOID i810_setscl(struct i2c_struct *st, UINT32 state)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	OUTREG(mmio + st->ddc_base, (state ? SCL_VAL_OUT : 0) | 
		SCL_DIR | SCL_DIR_MASK | SCL_VAL_MASK);
	val = INREG(mmio + st->ddc_base);
}

static UINT32 i810_getsda(struct i2c_struct *st)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	OUTREG(mmio + st->ddc_base, SDA_DIR_MASK);
	OUTREG(mmio + st->ddc_base, 0);
	val = INREG(mmio + st->ddc_base);
	return ((val & SDA_VAL_IN) != 0);
}

static UINT32 i810_getscl(struct i2c_struct *st)
{
	UCHAR *mmio = st->mmio_start_virtual;
	UINT32 val;

	OUTREG(mmio + st->ddc_base, SCL_DIR_MASK);
	OUTREG(mmio + st->ddc_base, 0);
	val = INREG(mmio + st->ddc_base);
	return ((val & SCL_VAL_IN) != 0);
}

//static VOID test(struct i2c_struct *st, char c)
//{
//	printf("\n\nTest %c\n", c);
//
//	st->setsda(st, HIGH);
//	st->setscl(st, HIGH);
//	Sleep(st->udelay);
//
//	printf("scl %c = 0x%08x\n", c, st->getscl(st));
//	printf("sda %c = 0x%08x\n", c, st->getsda(st));
//	Sleep(st->udelay);
//
//	st->setsda(st, LOW);
//	st->setscl(st, LOW);
//	Sleep(st->udelay);
//
//	printf("scl %c = 0x%08x\n", c, st->getscl(st));
//	printf("sda %c = 0x%08x\n", c, st->getsda(st));
//	Sleep(st->udelay);
//
//	st->setsda(st, HIGH);
//	st->setscl(st, HIGH);
//	Sleep(st->udelay);
//
//	printf("scl %c = 0x%08x\n", c, st->getscl(st));
//	printf("sda %c = 0x%08x\n", c, st->getsda(st));
//	Sleep(st->udelay);
//
//	st->setsda(st, LOW);
//	st->setscl(st, LOW);
//	Sleep(st->udelay);
//
//	printf("scl %c = 0x%08x\n", c, st->getscl(st));
//	printf("sda %c = 0x%08x\n", c, st->getsda(st));
//	Sleep(st->udelay);
//}

static VOID i810_init(struct i2c_struct *st)
{
	st->ddc_base = GPIOA;

	st->setsda = i810_setsda;
	st->setscl = i810_setscl;
	st->getsda = i810_getsda;
	st->getscl = i810_getscl;
	st->udelay = 10;

	/* Raise SCL and SDA */
	st->setsda(st, 1);
	st->setscl(st, 1);
	Sleep(20);

	//CHAR c;

	//st->ddc_base = GPIOA;
	//c = 'A';
	//test(c);

	//st->ddc_base = GPIOB;
	//c = 'B';
	//test(c);

	//st->ddc_base = GPIOC;
	//c = 'C';
	//test(c);

	//st->ddc_base = GPIOD;
	//c = 'D';
	//test(c);

	//st->ddc_base = GPIOE;
	//c = 'E';
	//test(c);

	//st->ddc_base = GPIOF;
	//c = 'F';
	//test(c);

	//st->ddc_base = GPIOG;
	//c = 'G';
	//test(c);

	//st->ddc_base = GPIOH;
	//c = 'H';
	//test(c);*/
}