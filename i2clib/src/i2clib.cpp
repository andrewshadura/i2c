// i2clib.cpp : Defines the exported functions for the DLL application.
//

#include <windows.h>
#include <stdio.h>
#include "..\external\pmdll.h"
#include "i2c_base.cpp"
#include "i810_i2c.cpp"
#include "radeon_i2c.cpp"

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

	struct i2c_struct *I2C_STRUCT = NULL;
	DWORD MMIO_PHYS_ADDRESS;
	DWORD MMIO_SIZE;

	__declspec(dllexport) BOOL __cdecl Open()
	{
		// get pci device MMIO_PHYS_ADDRESS & MMIO_SIZE
		// using pciutils

		MMIO_PHYS_ADDRESS = 0xFFA80000;	// Intel 82865G Graphics Controller
		MMIO_SIZE = 512 * 1024;

		if (LoadPhyMemDriver() == FALSE)
		{
			printf("\nload phymem.sys failed");

			return FALSE;
		}

		I2C_STRUCT = (struct i2c_struct *)calloc(1, sizeof(struct i2c_struct));

		I2C_STRUCT->mmio_start_virtual = (UINT8 *)MapPhyMem(MMIO_PHYS_ADDRESS, MMIO_SIZE);
		printf("\nmapped virtual address = 0x%08x", I2C_STRUCT->mmio_start_virtual);

		// check type
		i810_init(I2C_STRUCT);

		return TRUE;
	}

	__declspec(dllexport) VOID __cdecl Close()
	{
		if (I2C_STRUCT != NULL)
		{
			UnmapPhyMem(I2C_STRUCT->mmio_start_virtual, MMIO_SIZE);
		}

		UnloadPhyMemDriver();
	}

	__declspec(dllexport) BOOL __cdecl GetEdid(PUCHAR pEdidBuffer, ULONG EdidBufferSize)
	{
		if (I2C_STRUCT != NULL)
		{
			return DumpEdid(I2C_STRUCT, pEdidBuffer, EdidBufferSize);
		}
	}

#ifdef __cplusplus
}
#endif

