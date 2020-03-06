#include "common.h"

/*
# Name  : LoadImageNotifyRoutine
# Param : PUNICODE_STRING, HANDLE, PIMAGE_INFO
# Desc  : �̹����� �ε� �� �� �̹��� ����(�������,Ŀ�θ��)�� ���� ������ ���
*/
VOID LoadImageNotifyRoutine(IN PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo)
{
	if (!ImageInfo->SystemModeImage)
	{
		DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL,
			"[INFO] Load Image Name : \n\t[%.4X] %wZ\n", ProcessId, FullImageName);
	}

	else
	{
		DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL,
			"[INFO] Load Driver Name : \n\t[%.4X] %wZ\n", ProcessId, FullImageName);
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
