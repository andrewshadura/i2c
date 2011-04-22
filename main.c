#include <conio.h>
#include <stdio.h>
#include <Windows.h>
#include "pmdll.h"
//#include "pci_codes.h"

/* General Defines */
#define MMIO_SIZE                   (512*1024)

/* I/O Control Registers (05000h 05FFFh) */
#define HVSYNC                0x05000 
#define GPIOA                 0x05010	//	"I810-DDC"
#define GPIOB                 0x05014	//	"I810-I2C" 
#define GPIOC                 0x0501C	//	"I810-GPIOC"

/* 
 * Register I/O
 */
#define i810_readb(where, mmio) readb(mmio + where)
#define i810_readw(where, mmio) readw(mmio + where)
#define i810_readl(where, mmio) readl(mmio + where)
#define i810_writeb(where, mmio, val) writeb(val, mmio + where) 
#define i810_writew(where, mmio, val) writew(val, mmio + where)
#define i810_writel(where, mmio, val) writel(val, mmio + where)

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
UINT32 mmio_start_phys = 0xD0200000;
VOID *mmio_start_virtual;
VOID *mmio;

VOID i810i2c_setscl(int state)
{
	if (state)
		i810_writel(mmio, ddc_base, SCL_DIR_MASK | SCL_VAL_MASK);
	else
		i810_writel(mmio, ddc_base, SCL_DIR | SCL_DIR_MASK | SCL_VAL_MASK);
	i810_readl(mmio, ddc_base);	/* flush posted write */
}

VOID i810i2c_setsda(int state)
{
	if (state)
		i810_writel(*mmio, ddc_base, SDA_DIR_MASK | SDA_VAL_MASK);
	else
		i810_writel(*mmio, ddc_base, SDA_DIR | SDA_DIR_MASK | SDA_VAL_MASK);
	i810_readl(*mmio, ddc_base);	/* flush posted write */
}

UINT32 i810i2c_getscl()
{
	i810_writel(*mmio, ddc_base, SCL_DIR_MASK);
	i810_writel(*mmio, ddc_base, 0);
	return ((i810_readl(*mmio, ddc_base) & SCL_VAL_IN) != 0);
}

UINT32 i810i2c_getsda()
{
	i810_writel(*mmio, ddc_base, SDA_DIR_MASK);
	i810_writel(*mmio, ddc_base, 0);
	return ((i810_readl(*mmio, ddc_base) & SDA_VAL_IN) != 0);
}

//http://lwn.net/Articles/102232/
//http://www.fiveanddime.net/kernel/linux-2.6.11.2/arch/alpha/kernel/io.c.html

//UINT32 CheckPci(UCHAR bus, UCHAR devc, UCHAR func, UCHAR reg);
//VOID ListPci();

int main(int argc, char* argv[])
{
	BOOL b = LoadPhyMemDriver();

	if (b == FALSE)
	{
		printf("load phymem.sys failed\n");
		exit(-1);
	}

	mmio_start_virtual = (VOID*)MapPhyMem(mmio_start_phys, MMIO_SIZE);
	printf("mapped virtual address = 0x%08x\n", (CHAR *)mmio_start_virtual);

	//ddc_base = GPIOA; //"I810-DDC"
	ddc_base = GPIOB; //"I810-I2C"
	//ddc_base = GPIOC; //"I810-GPIOC");

	mmio = mmio_start_virtual;

	UnmapPhyMem(mmio_start_virtual, MMIO_SIZE);

	UnloadPhyMemDriver();

	//ListPci();

	printf("end\n");
	getchar();
	
	return 0;
}

//UINT32 CheckPci(UCHAR bus, UCHAR devc, UCHAR func, UCHAR reg)
//{
//	UINT32 temp = 0;
//
//	__asm
//	{
//		xor eax, eax
//			mov al, bus
//			or ah, 80h
//			shl eax, 16
//			mov ah, devc
//			shl ah, 3
//			or ah, func
//			mov al, reg
//			and al, 0FCh
//
//			mov temp, eax
//	}
//
//	WritePortLong(0x0CF8, temp);
//	temp = ReadPortLong(0x0CFC);
//
//	return temp;
//}
//
//VOID ListPci()
//{
//	UCHAR bus = 0, devc = 0, func = 0, base_class = 0, sub_class = 0, prog_in = 0;
//	USHORT vendor_id = 0, device_id = 0;
//	UINT32 temp = 0;
//	printf("%-5s %-5s %-5s %5s %12s %24s %10s %20s %60s %25s %18s %7s\n", 
//		"Bus", "Dev", "Func", "VenID", "VenShort", "VenFull", "DevId", "Chip", 
//		"ChipDesc", "BaseClass", "SubClass", "ProIn");
//	for (int i = 0; i < 210; i++)
//	{
//		printf("-");
//	}
//	printf("\n");
//
//	do
//	{
//		temp = CheckPci(bus, devc, func, 0);
//
//		if (temp != 0xFFFFFFFF)
//		{
//			vendor_id = (USHORT) temp;
//			device_id = (USHORT) (temp >> 16);
//
//			temp = CheckPci(bus, devc, func, 8);
//
//			base_class = (UCHAR) (temp >> 24);
//			sub_class = (UCHAR) (temp >> 16) & 0x0F;
//			prog_in = (UCHAR) (temp >> 8) & 0x0F;
//
//			//if (base_class == 0x01)	// выбор класса оборудования, жесткий диск
//			{
//				printf("%-5.02x %-5.02x %-5.02x", bus, devc, func);
//
//				for (int i = 0; i != PCI_VENTABLE_LEN; i++)
//				{
//					if (PciVenTable[i].VenId == vendor_id)
//					{
//						printf(" %5.05x %12s %24s", PciVenTable[i].VenId, PciVenTable[i].VenShort, 
//							PciVenTable[i].VenFull);
//					}
//				}
//
//				int k = 0;
//				for (int i = 0;  i != PCI_DEVTABLE_LEN; i++)
//				{
//					if (PciDevTable[i].VenId == vendor_id && 
//						PciDevTable[i].DevId == device_id)
//					{
//						printf(" %10.05x %20s %60s", PciDevTable[i].DevId, 
//							PciDevTable[i].Chip, PciDevTable[i].ChipDesc);
//
//						k = 1;
//					}
//				}
//				if (k == 0)
//				{
//					printf(" %10s %20s %60s", "-", "-", "-");
//				}
//
//				k = 0;
//				for (int i = 0; i != PCI_CLASSCODETABLE_LEN; i++)
//				{
//					if (PciClassCodeTable[i].BaseClass == base_class &&
//						PciClassCodeTable[i].SubClass == sub_class && 
//						PciClassCodeTable[i].ProgIf == prog_in)
//					{
//						printf(" %25s %18s %7s", PciClassCodeTable[i].BaseDesc, 
//							PciClassCodeTable[i].SubDesc, 
//							PciClassCodeTable[i].ProgDesc);
//
//						k = 1;
//					}
//				}
//				if (k == 0)
//				{
//					printf(" %25s %18s %7s", "-", "-", "-");
//				}
//
//				printf("\n");
//			}
//		}
//
//		func++;
//		if (func != 8)
//		{
//			continue;
//		}
//		else
//		{
//			func = 0;
//			devc++;
//			if (devc != 32)
//			{
//				continue;
//			}
//			else
//			{
//				devc = 0;
//				bus++;
//			}
//		}
//	}
//	while (bus != 255);
//
//	for (int i = 0; i < 210; i++)
//	{
//		printf("-");
//	}
//	printf("\n");
//}