#include "control.h"

/*//////////////////////////////////////////////
# File : main.c
# Desc : 드라이버 진입점, 해제 루틴 정의
*///////////////////////////////////////////////


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath)
{
	UNREFERENCED_PARAMETER(pRegPath);

	UNICODE_STRING deviceName = { 0, };

	RtlInitUnicodeString(&deviceName, DeviceName);
	RtlInitUnicodeString(&SymbolickLink, L"\\??\\ControlDebugger");

	if (GetOffset(PsGetCurrentProcess()))
	{
		if (IoCreateDevice(
			pDriver, 0, &deviceName, FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN, FALSE, &pDevice) == STATUS_SUCCESS)
		{
			IoCreateSymbolicLink(&SymbolickLink, &deviceName);
		}
	}
	pDriver->DriverUnload = UnloadDriver;
	pDriver->MajorFunction[IRP_MJ_CREATE] = ControlDebugger;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlDebugger;
	
	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT pDriver)
{
	UNREFERENCED_PARAMETER(pDriver);
	if (MmIsAddressValid(pDevice))
	{
		IoDeleteDevice(pDevice);
		IoDeleteSymbolicLink(&SymbolickLink);
	}

}