// test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>

typedef BOOL (*OpenFunc)();
typedef VOID (*CloseFunc)();
typedef BOOL (*GetEdidFunc)(PUCHAR pEdidBuffer, ULONG EdidBufferSize);

INT32 main(INT32 argc, CHAR* argv[])
{
	HINSTANCE hInstLibrary = LoadLibrary(TEXT("i2clib.dll"));
	OpenFunc _OpenFunc;
	CloseFunc _CloseFunc;
	GetEdidFunc _GetEdidFunc;

	if (hInstLibrary)
	{
		_OpenFunc = (OpenFunc)GetProcAddress(hInstLibrary, "Open");
		_CloseFunc = (CloseFunc)GetProcAddress(hInstLibrary, "Close");
		_GetEdidFunc = (GetEdidFunc)GetProcAddress(hInstLibrary, "GetEdid");

		if (_OpenFunc)
		{
			printf("\n_OpenFunc found");

			if (_OpenFunc())
			{
				if (_GetEdidFunc)
				{
					printf("\n_GetEdidFunc found");

					PUCHAR edid = (PUCHAR)calloc(128, sizeof(UCHAR));
					if (_GetEdidFunc(edid, 128))
					{
						printf("\nEDID:\n");
						for (int i = 0; i < 128; i++)
							printf("0x%02x ", edid[i]);
					}
				}
			}
		}
		
		if (_CloseFunc)
		{
			printf("\n_CloseFunc found");

			_CloseFunc();
		}

		FreeLibrary(hInstLibrary);
	}
	else
	{
		printf("DLL Failed To Load!");
	}

	printf("\n\nEND");
	getchar();

	return 0;
}

