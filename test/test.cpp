#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "pmdll.h"

#define MMIO_SIZE                   (512*1024)

#define GPIOA             0x5010
#define GPIOB             0x5014
#define GPIOC             0x5018 /* this may be external DDC on i830 */
#define GPIOD             0x501C /* this is DVO DDC */
#define GPIOE             0x5020 /* this is DVO i2C */
#define GPIOF             0x5024
#define GPIOG		      0x5028
#define GPIOH			  0x502c

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

UINT32 ddc_base;
UINT32 mmio_start_phys;
UCHAR *mmio_start_virtual;
UCHAR *mmio;

#define OUTREG(reg, state) (*((PUINT32)(reg)) = state)
#define INREG(reg) (*((PUINT32)(reg)))

static void WRITE_SCL(int state)
{
	UINT32 val;

	OUTREG(mmio + ddc_base, (state ? SCL_VAL_OUT : 0) | 
		SCL_DIR | SCL_DIR_MASK | SCL_VAL_MASK);
	val = INREG(mmio + ddc_base);
}

static void WRITE_SDA(int state)
{
	UINT32 val;

	OUTREG(mmio + ddc_base, (state ? SDA_VAL_OUT : 0) |
		SDA_DIR | SDA_DIR_MASK | SDA_VAL_MASK);
	val = INREG(mmio + ddc_base);
}

static UINT32 READ_SCL()
{
	UINT32 val;

	OUTREG(mmio + ddc_base, SCL_DIR_MASK);
	OUTREG(mmio + ddc_base, 0);
	val = INREG(mmio + ddc_base);
	return ((val & SCL_VAL_IN) != 0);
}

static UINT32 READ_SDA()
{
	UINT32 val;

	OUTREG(mmio + ddc_base, SDA_DIR_MASK);
	OUTREG(mmio + ddc_base, 0);
	val = INREG(mmio + ddc_base);
	return ((val & SDA_VAL_IN) != 0);
}

#define LOW               0
#define HIGH              1
#define WRITE             0
#define READ              1

#define DELAY_HALF()      Sleep(20);

BOOL I2CWrite(UCHAR Data)
{
	UCHAR Bit;
	BOOL Ack;

	/* transmit data */
	for (Bit = (1 << 7); Bit != 0; Bit >>= 1)
	{
		WRITE_SCL(LOW);
		WRITE_SDA((Data & Bit) ? HIGH : LOW);
		DELAY_HALF();
		WRITE_SCL(HIGH);
		DELAY_HALF();
	}

	/* get ack */
	WRITE_SCL(LOW);
	WRITE_SDA(HIGH);
	DELAY_HALF();
	WRITE_SCL(HIGH);
	do
	{
		DELAY_HALF();
	}
	while (READ_SCL() != HIGH);
	Ack = (READ_SDA() == LOW);
	DELAY_HALF();

	printf("I2CWrite: %s\n", Ack ? "Ack" : "Nak");
	return Ack;
}

UCHAR I2CRead(BOOL Ack)
{
	INT Bit = 0x80;
	UCHAR Data = 0;

	/* pull down SCL and release SDA */
	WRITE_SCL(LOW);
	WRITE_SDA(HIGH);

	/* read byte */
	for (Bit = (1 << 7); Bit != 0; Bit >>= 1)
	{
		WRITE_SCL(LOW);
		DELAY_HALF();
		WRITE_SCL(HIGH);
		DELAY_HALF();
		if (READ_SDA() == HIGH)
			Data |= Bit;
	}

	/* send ack/nak */
	WRITE_SCL(LOW);
	WRITE_SDA(Ack ? LOW : HIGH);
	DELAY_HALF();
	WRITE_SCL(HIGH);
	do
	{
		DELAY_HALF();
	}
	while (READ_SCL() != HIGH);

	return Data;
}

VOID I2CStop()
{
	WRITE_SCL(LOW);
	WRITE_SDA(LOW);
	DELAY_HALF();
	WRITE_SCL(HIGH);
	DELAY_HALF();
	WRITE_SDA(HIGH);
}

BOOL I2CStart(UCHAR Address)
{
	/* make sure the bus is free */
	if (READ_SDA() == LOW || READ_SCL() == LOW)
	{
		printf("I2CStart: Bus is not free!\n");
		return FALSE;
	}

	/* send address */
	WRITE_SDA(LOW);
	DELAY_HALF();
	if (!I2CWrite(Address))
	{
		/* ??release the bus?? */
		I2CStop();
		printf("I2CStart: Device not found (Address = 0x%x)\n", Address);
		return FALSE;
	}

	printf("I2CStart: SUCCESS!\n");
	return TRUE;
}

BOOL I2CRepStart(UCHAR Address)
{
	/* setup lines for repeated start condition */
	WRITE_SCL(LOW);
	DELAY_HALF();
	WRITE_SDA(HIGH);
	DELAY_HALF();
	WRITE_SCL(HIGH);
	DELAY_HALF();

	return I2CStart(Address);
}

BOOLEAN GetEdid(PUCHAR pEdidBuffer, ULONG EdidBufferSize);


VOID Test(char c)
{
	printf("\n\nTest %c\n", c);

	WRITE_SDA(HIGH);
	WRITE_SCL(HIGH);
	DELAY_HALF();

	printf("scl %c = 0x%08x\n", c, READ_SCL());
	printf("sda %c = 0x%08x\n", c, READ_SDA());
	DELAY_HALF();

	WRITE_SDA(LOW);
	WRITE_SCL(LOW);
	DELAY_HALF();

	printf("scl %c = 0x%08x\n", c, READ_SCL());
	printf("sda %c = 0x%08x\n", c, READ_SDA());
	DELAY_HALF();

	WRITE_SDA(HIGH);
	WRITE_SCL(HIGH);
	DELAY_HALF();

	printf("scl %c = 0x%08x\n", c, READ_SCL());
	printf("sda %c = 0x%08x\n", c, READ_SDA());
	DELAY_HALF();

	WRITE_SDA(LOW);
	WRITE_SCL(LOW);
	DELAY_HALF();

	printf("scl %c = 0x%08x\n", c, READ_SCL());
	printf("sda %c = 0x%08x\n", c, READ_SDA());
	DELAY_HALF();
}

/* XP SP2, Intel 82865G Graphics Controller */
#define DDC_EEPROM_ADDRESS  0xA0
#define MMIO_START_PHYS		0xFFA80000

int main()
{
	CHAR c;
	BOOL b = LoadPhyMemDriver();

	if (b == FALSE)
	{
		printf("load phymem.sys failed\n");
		return 1;
	}

	UINT8 *mmio_start_virtual = (UINT8 *)MapPhyMem(MMIO_START_PHYS, MMIO_SIZE);
	printf("mapped virtual address = 0x%08x\n", mmio_start_virtual);

	mmio = mmio_start_virtual;
	ddc_base = GPIOA;

	WRITE_SDA(HIGH);
	WRITE_SCL(HIGH);
	DELAY_HALF();

	PUCHAR edid = (PUCHAR)calloc(128, sizeof(UCHAR));
	if (GetEdid(edid, 128))
	{
		printf("\nEDID:\n");
		for (int i = 0; i < 128; i++)
			printf("0x%02x ", edid[i]);
	}

	///*ddc_base = GPIOA;
	//c = 'A';
	//Test(c);

	//ddc_base = GPIOB;
	//c = 'B';
	//Test(c);

	//ddc_base = GPIOC;
	//c = 'C';
	//Test(c);

	//ddc_base = GPIOD;
	//c = 'D';
	//Test(c);

	//ddc_base = GPIOE;
	//c = 'E';
	//Test(c);

	//ddc_base = GPIOF;
	//c = 'F';
	//Test(c);

	//ddc_base = GPIOG;
	//c = 'G';
	//Test(c);

	//ddc_base = GPIOH;
	//c = 'H';
	//Test(c);*/

	UnmapPhyMem(mmio_start_virtual, MMIO_SIZE);
	UnloadPhyMemDriver();

	printf("\nend");
	getchar();

	return 0;
}

BOOLEAN GetEdid(PUCHAR pEdidBuffer, ULONG EdidBufferSize)
{
	INT Count, i;
	PUCHAR pBuffer = (PUCHAR)pEdidBuffer;
	BOOL Ack;

	printf("GetEdid()\n");

	/* select eeprom */
	if (!I2CStart(DDC_EEPROM_ADDRESS | WRITE))
		return FALSE;
	/* set address */
	if (!I2CWrite(0x00))
		return FALSE;
	/* change into read mode */
	if (!I2CRepStart(DDC_EEPROM_ADDRESS | READ))
		return FALSE;
	/* read eeprom */
	RtlZeroMemory(pEdidBuffer, EdidBufferSize);
	Count = min(128, EdidBufferSize);
	for (i = 0; i < Count; i++)
	{
		Ack = ((i + 1) < Count);
		pBuffer[i] = I2CRead(Ack);
	}
	I2CStop();

	/* check EDID header */
	if (pBuffer[0] != 0x00 || pBuffer[1] != 0xff ||
		pBuffer[2] != 0xff || pBuffer[3] != 0xff ||
		pBuffer[4] != 0xff || pBuffer[5] != 0xff ||
		pBuffer[6] != 0xff || pBuffer[7] != 0x00)
	{
		printf("GetEdid(): Invalid EDID header!\n");
		return FALSE;
	}

	printf("GetEdid(): EDID version %d rev. %d\n", pBuffer[18], pBuffer[19]);
	printf("GetEdid() - SUCCESS!\n");
	return TRUE;
}