#include "callbacks.h"

/*//////////////////////////////////////////////////////
# Scenario : ObRegisterCallbacks �� PsSetLoadImageNotifyRoutine �� �̿��� ��Ƽ �����
	- ObRegisterCallbacks : PEB �� �̿��ؼ� ������� ����� ������ Ž��
	- PsSetLoadImageNotifyRoutine : Ŀ�� ����� ������ Ȯ��(KdDebuggerEnabeld, KdDebuggerNotPresent ��..)
# File : main.c
# Desc : ����̹� �������� ���� ��ƾ, ����� ���� �Լ�
*///////////////////////////////////////////////////////


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriver, IN PUNICODE_STRING pRegPath)
{
	UNREFERENCED_PARAMETER(pRegPath);
	UNICODE_STRING PsGetProcessDebugPortString = { 0, };
	pDriver->DriverUnload = UnloadDriver;
	DbgPrintEx(DPFLTR_ACPI_ID, 3, "[INFO] Driver load success\n");

	if (GetOffset(PsGetCurrentProcess()))
	{
		RtlCreateUnicodeString(&PsGetProcessDebugPortString, L"PsGetProcessDebugPort");
		PsGetProcessDebugPort = (PsGetProcessDebugPort_t)MmGetSystemRoutineAddress(&PsGetProcessDebugPortString);

		if (ObCallbackReg() == STATUS_SUCCESS)
		{
			PsSetLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
		}
	}
	return STATUS_SUCCESS;
}

VOID UnloadDriver(IN PDRIVER_OBJECT pDriver)
{
	UNREFERENCED_PARAMETER(pDriver);
	
	PsRemoveLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
	if (hRegistration)	// �ݹ� ��Ͽ� ������ ��� ���� ó��
	{
		ObUnRegisterCallbacks(hRegistration);
	}
	
	DbgPrintEx(DPFLTR_ACPI_ID, 3, "[INFO] Driver unload success\n");
}