#pragma once
#include "common.h"

/*
# Name  : Dummy
# Param : x
# Desc  : 콜백 루틴 덮어쓰기용 더미 함수
*/
VOID Dummy()
{

}

/*
# Name  : GetOffset
# Param : PEPROCESS
# Desc  : EPROCESS 구조체의 특정 멤버 오프셋 구하는 함수
*/
BOOLEAN GetOffset(PEPROCESS Process)
{
	BOOLEAN success = FALSE;
	HANDLE PID = PsGetCurrentProcessId();
	PLIST_ENTRY ListEntry = { 0, };
	PLIST_ENTRY NextEntry = { 0, };

	for (int i = 0x80; i < PAGE_SIZE - 0x10; i += 4)
	{
		if (*(PHANDLE)((PCHAR)Process + i) == PID)
		{
			ListEntry = (PVOID*)((PCHAR)Process + i + 0x8);
			if (MmIsAddressValid(ListEntry) && MmIsAddressValid(ListEntry->Flink))
			{
				NextEntry = ListEntry->Flink;
				if (ListEntry == NextEntry->Blink)
				{
					iOffset.UniqueProcessid_off = i;
					iOffset.ActiveProcessLinks_off = i + 8;
					success = TRUE;
					break;
				}
			}
		}
	}
	if (!success)
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[ERR] Not found offset\n");
		return success;
	}

	// ImageFileName Offset 
	success = FALSE;
	for (int i = iOffset.ActiveProcessLinks_off; i < PAGE_SIZE; i++)
	{
		if (!strncmp((PCHAR)Process + i, szSystem, 6))
		{
			iOffset.ImageFileName_off = i;
			success = TRUE;
			break;
		}
	}
	if (!success)
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[ERR] Not found offset\n");
		return success;
	}

	if (!GetPebOffset())
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[ERR] Not found offset\n");
		return success;
	}
	return success;
}

/*
# Name  : GetPebOffset
# Param : x
# Desc  : EPROCESS 구조체 내 PEB 오프셋 구하는 함수
*/
BOOLEAN GetPebOffset()
{
	int LinkOffset = iOffset.ActiveProcessLinks_off;
	int ProcName = iOffset.ImageFileName_off;
	BOOLEAN success = FALSE;
	PEPROCESS Process = PsGetCurrentProcess();
	UNICODE_STRING routineName = { 0, };

	RtlInitUnicodeString(&routineName, szNtQueryInformationProcess);
	NtQueryInformationProcess_t NtQueryInformationProcess = MmGetSystemRoutineAddress(&routineName);

	for (int i = 0; i < 0x10; i++)
	{
		PROCESS_BASIC_INFORMATION ProcessInformation = { 0, };
		PLIST_ENTRY ListEntry = (PVOID*)((PCHAR)Process + LinkOffset);
		Process = ((PCHAR)ListEntry->Flink - LinkOffset);
		HANDLE Key = NULL;

		if (ObOpenObjectByPointer(Process, NULL, NULL, NULL, *PsProcessType, KernelMode, &Key)
			== STATUS_SUCCESS)
		{
			PULONG Ret = NULL;
			NtQueryInformationProcess(
				Key, ProcessBasicInformation, &ProcessInformation, sizeof(ProcessInformation), Ret);

			ZwClose(Key);
		}

		if (ProcessInformation.PebBaseAddress)
		{
			for (int j = iOffset.ActiveProcessLinks_off; j < PAGE_SIZE - 0x10; j += 4)
			{
				if (*(PHANDLE)((PCHAR)Process + j) == ProcessInformation.PebBaseAddress)
				{
					iOffset.PEB_off = j;
					success = TRUE;
					return success;
				}
			}
		}
	}
	return success;
}

/*
# Name  : GetModuleInformation
# Param : const char*
# Desc  : GetModuleHandle 커널 모드 버전
*/
NTSTATUS GetModuleInformation(const char* szModuleName)
{
	BOOLEAN tmpSwitch = FALSE;
	ULONG infoLen = 0;
	UNICODE_STRING ZwQueryString = { 0, };
	PSYSTEM_MODULE_INFORMATION pMod = { 0, };
	RtlInitUnicodeString(&ZwQueryString, L"ZwQuerySystemInformation");
	NtQuerySystemInformation_t ZwQuerySystemInformation = MmGetSystemRoutineAddress(&ZwQueryString);

	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, &infoLen, 0, &infoLen);
	pMod = (PSYSTEM_MODULE_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, infoLen, 'H0YA');
	RtlZeroMemory(pMod, infoLen);
	status = ZwQuerySystemInformation(SystemModuleInformation, pMod, infoLen, &infoLen);
	PSYSTEM_MODULE_ENTRY pModEntry = pMod->Module;
	for (int i = 0; i < pMod->Count; i++)
	{
		if (!_stricmp(pModEntry[i].FullPathName, szModuleName))
		{
			DbgPrint("[+] Find Module %s\n", pModEntry[i].FullPathName);
			TargetModule = pModEntry[i];
			tmpSwitch = TRUE;
			break;
		}
	}
	ExFreePoolWithTag(pMod, 'H0YA');
	if (!tmpSwitch)
	{
		return STATUS_NOT_FOUND;
	}
	return status;
}