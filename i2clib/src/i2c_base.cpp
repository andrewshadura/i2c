#include "i2c.h"

#define DDC_EEPROM_ADDRESS  0xA0

#define WRITE             0
#define READ              1

static BOOL I2CWrite(struct i2c_struct *st, UCHAR Data)
{
	UCHAR Bit;
	BOOL Ack;

	/* transmit data */
	for (Bit = (1 << 7); Bit != 0; Bit >>= 1)
	{
		st->setscl(st, LOW);
		st->setsda(st, (Data & Bit) ? HIGH : LOW);
		Sleep(st->udelay);
		st->setscl(st, HIGH);
		Sleep(st->udelay);
	}

	/* get ack */
	st->setscl(st, LOW);
	st->setsda(st, HIGH);
	Sleep(st->udelay);
	st->setscl(st, HIGH);
	do
	{
		Sleep(st->udelay);
	}
	while (st->getscl(st) != HIGH);
	Ack = (st->getsda(st) == LOW);
	Sleep(st->udelay);

	printf("I2CWrite: %s\n", Ack ? "Ack" : "Nak");
	return Ack;
}

static UCHAR I2CRead(struct i2c_struct *st, BOOL Ack)
{
	INT Bit = 0x80;
	UCHAR Data = 0;

	/* pull down SCL and release SDA */
	st->setscl(st, LOW);
	st->setsda(st, HIGH);

	/* read byte */
	for (Bit = (1 << 7); Bit != 0; Bit >>= 1)
	{
		st->setscl(st, LOW);
		Sleep(st->udelay);
		st->setscl(st, HIGH);
		Sleep(st->udelay);
		if (st->getsda(st) == HIGH)
			Data |= Bit;
	}

	/* send ack/nak */
	st->setscl(st, LOW);
	st->setsda(st, Ack ? LOW : HIGH);
	Sleep(st->udelay);
	st->setscl(st, HIGH);
	do
	{
		Sleep(st->udelay);
	}
	while (st->getscl(st) != HIGH);

	return Data;
}

static VOID I2CStop(struct i2c_struct *st)
{
	st->setscl(st, LOW);
	st->setsda(st, LOW);
	Sleep(st->udelay);
	st->setscl(st, HIGH);
	Sleep(st->udelay);
	st->setsda(st, HIGH);
}

static BOOL I2CStart(struct i2c_struct *st, UCHAR Address)
{
	/* make sure the bus is free */
	if (st->getsda(st) == LOW || st->getscl(st) == LOW)
	{
		printf("I2CStart: Bus is not free!\n");
		return FALSE;
	}

	/* send address */
	st->setsda(st, LOW);
	Sleep(st->udelay);
	if (!I2CWrite(st, Address))
	{
		/* ??release the bus?? */
		I2CStop(st);
		printf("I2CStart: Device not found (Address = 0x%x)\n", Address);
		return FALSE;
	}

	printf("I2CStart: SUCCESS!\n");
	return TRUE;
}

static BOOL I2CRepStart(struct i2c_struct *st, UCHAR Address)
{
	/* setup lines for repeated start condition */
	st->setscl(st, LOW);
	Sleep(st->udelay);
	st->setsda(st, HIGH);
	Sleep(st->udelay);
	st->setscl(st, HIGH);
	Sleep(st->udelay);

	return I2CStart(st, Address);
}

static BOOLEAN DumpEdid(struct i2c_struct *st, PUCHAR pEdidBuffer, ULONG EdidBufferSize)
{
	INT Count, i;
	PUCHAR pBuffer = (PUCHAR)pEdidBuffer;
	BOOL Ack;

	printf("GetEdid()\n");

	/* select eeprom */
	if (!I2CStart(st, DDC_EEPROM_ADDRESS | WRITE))
		return FALSE;
	/* set address */
	if (!I2CWrite(st, 0x00))
		return FALSE;
	/* change into read mode */
	if (!I2CRepStart(st, DDC_EEPROM_ADDRESS | READ))
		return FALSE;
	/* read eeprom */
	RtlZeroMemory(pEdidBuffer, EdidBufferSize);
	Count = min(128, EdidBufferSize);
	for (i = 0; i < Count; i++)
	{
		Ack = ((i + 1) < Count);
		pBuffer[i] = I2CRead(st, Ack);
	}
	I2CStop(st);

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