// l1tf_victim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <intrin.h>

int main()
{
	int c[4];
	int ProcessorNumber = -1;

	printf("Victim Process: %u\r\n", GetCurrentProcessId());

	// set current process affinity to 0
	if (SetProcessAffinityMask(GetCurrentProcess(), 1 << 0))
	{
		printf( "[+] process affinity set\n" );
	}
	else
	{
		printf("[-] unable to set process affinity\n");
		return 0;
	}

	__rdtscp((unsigned int *)&ProcessorNumber);

	printf("[*] Process running on precessor: %d\n", ProcessorNumber);

	HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_NOCACHE | SEC_COMMIT, 0, 0x1000, "VictimSharedUncachedMemory");

	if (hMap == NULL)
	{
		printf("[-] unable to create file mapping\n");
		return 0;
	}

	volatile PUCHAR pMem = (PUCHAR)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000);

	if (pMem == NULL)
	{
		printf("[-] unable to map shared memory\n");
		return 0;
	}
	else
	{
		printf("[+] uncached shared memory mapped: %p\n", pMem);

	}

	//
	printf("[*] waiting on attacker process ...\n");
	while (1)
	{
		if (pMem[0] == 1)
		{
			Sleep(1000);
			break;
		}
		Sleep(100);
	}	
	const char Secret[] = "GenuineYOLO";

	printf("placing secret into unchaced shared memory: %s\n", Secret);

	int nmin = 0;
	int nsec = 0, nmsec = 0;

	while (1)
	{		
		// SECRET SAUCE
		pMem[0]  = 'G';
		pMem[1]  = 'e';
		pMem[2]  = 'n';
		pMem[3]  = 'u';
		pMem[4]  = 'i';
		pMem[5]  = 'n';
		pMem[6]  = 'e';
		pMem[7]  = 'Y';
		pMem[8]  = 'O';
		pMem[9]  = 'L';
		pMem[10] = 'O';
		
		//Sleep(1);
		//nmsec++;
		//if( nmsec == 1000 )
		//{
		//	nmsec = 0;
		//	nsec++;
		//	printf("+");
		//}
		//if (nsec == 60)
		//{
		//	nsec = 0;
		//	nmin++;
		//	printf(" %d\n", nmin );
		//}
	}

    return 0;
}

