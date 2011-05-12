#include "i810_i2c.cpp"
#include "i2c_base.cpp"
#include "pmdll.h"

int main()
{
	// get pci device MMIO_PHYS_ADDRESS & MMIO_SIZE
	// using pciutils

	DWORD MMIO_PHYS_ADDRESS = 0xFFA80000;	// Intel 82865G Graphics Controller
	DWORD MMIO_SIZE = 512 * 1024;

	if (LoadPhyMemDriver() == FALSE)
	{
		printf("load phymem.sys failed\n");
		return 1;
	}

	struct i2c_struct *st = (struct i2c_struct *)calloc(1, sizeof(struct i2c_struct));

	st->mmio_start_virtual = (UINT8 *)MapPhyMem(MMIO_PHYS_ADDRESS, MMIO_SIZE);
	printf("mapped virtual address = 0x%08x\n", st->mmio_start_virtual);

	// check type
	i810_init(st);

	PUCHAR edid = (PUCHAR)calloc(128, sizeof(UCHAR));
	if (GetEdid(st, edid, 128))
	{
		printf("\nEDID:\n");
		for (int i = 0; i < 128; i++)
			printf("0x%02x ", edid[i]);
	}

	UnmapPhyMem(st->mmio_start_virtual, MMIO_SIZE);
	UnloadPhyMemDriver();

	printf("\n\nend");
	getchar();

	return 0;
}