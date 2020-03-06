#include "common.h"

/*
# Name  : TerminateProcess
# Param : HANDLE
# Desc  : PID�� ���μ��� �ڵ��� ���� ��, ���� ���μ��� ����
*/
VOID TerminateProcess(IN HANDLE pid)
{
	HANDLE hProcess = NULL;
	OBJECT_ATTRIBUTES obAttr = { 0, };
	CLIENT_ID cid = { 0, };

	obAttr.Length = sizeof(obAttr);
	obAttr.Attributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
	cid.UniqueProcess = pid;

	if (ZwOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &obAttr, &cid) == STATUS_SUCCESS)
	{
		if (ZwTerminateProcess(hProcess, STATUS_ACCESS_DENIED) == STATUS_SUCCESS)
		{
			DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_WARNING_LEVEL,
				"[INFO] Success terminate process\n");
		}
		else
		{
			DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_WARNING_LEVEL,
				"[ERROR] Failed terminate process\n");
		}
	}
	else
	{
		DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_WARNING_LEVEL,
			"[ERROR] Failed open process\n");
	}


}

/*
# Name  : LoadImageNotifyRoutine
# Param : PUNICODE_STRING, HANDLE, PIMAGE_INFO
# Desc  : �� ����Ʈ�� ��� �� �̹����� �ε� �� �� TerminateProcess �Լ��� ȣ��
*/
VOID LoadImageNotifyRoutine(IN PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo)
{
	if (!ImageInfo->SystemModeImage)
	{
		for (int i = 0; i < sizeof(szTarget) / sizeof(PVOID); i++)
		{
			if (wcsstr(FullImageName->Buffer, szTarget[i]))
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_WARNING_LEVEL,
					"[WARN] Unauthorized Image Load : \n\t[%.4X] %wZ\n", ProcessId, FullImageName);

				TerminateProcess(ProcessId);
				
			}
		}

	}
}

/*
# Name  : DriverEntry
# Param : PDRIVER_OBJECT, PUNICODE_STRING
# Desc  : ����̹� ������
*/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriver, IN PUNICODE_STRING pRegPath)
{
	UNREFERENCED_PARAMETER(pRegPath);

	pDriver->DriverUnload = UnloadDriver;

	DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Load Driver\n");

	if (PsSetLoadImageNotifyRoutine(&LoadImageNotifyRoutine) != STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Failed register\n");

	}
	else
	{
		DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Success register\n");
	}

	return STATUS_SUCCESS;
}

/*
# Name  : UnloadDriver
# Param : PDRIVER_OBJECT
# Desc  : ����̹� ���� ��ƾ, ��ϵ� �ݹ� ��ƾ�� ����
*/
VOID UnloadDriver(IN PDRIVER_OBJECT pDriver)
{
	UNREFERENCED_PARAMETER(pDriver);
	PsRemoveLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
	DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Unload Driver\n");

}
