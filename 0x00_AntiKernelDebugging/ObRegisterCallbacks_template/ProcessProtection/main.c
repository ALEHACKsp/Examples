#include "callbacks.h"

PVOID hRegistration = NULL;	// ��ε� ��, ����ϱ� ���� ���������� ����

/*
# Name  : ObRegExample
# Param : x
# Desc  : OB_CALLBACK, OPERATION_REGISTRATION ����ü �ʱ�ȭ �� ObRegisterCallbacks �� �̿��� �ݹ� ��ƾ ���
*/
NTSTATUS ObRegExample()
{
	OB_CALLBACK_REGISTRATION obRegistration = { 0, };
	OB_OPERATION_REGISTRATION opRegistration = { 0, };

	obRegistration.Version = ObGetFilterVersion();
	obRegistration.OperationRegistrationCount = 1;
	RtlInitUnicodeString(&obRegistration.Altitude, L"300000");
	obRegistration.RegistrationContext = NULL;

	opRegistration.ObjectType = PsProcessType;
	opRegistration.Operations = OB_OPERATION_HANDLE_CREATE;
	opRegistration.PreOperation = PreCallback;
	opRegistration.PostOperation = PostCallback;

	obRegistration.OperationRegistration = &opRegistration;
	
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] ObRegisterCallbacks Test\n");
	return ObRegisterCallbacks(&obRegistration,&hRegistration);
}

/*
# Name  : DriverEntry
# Param : PDRIVER_OBJECT, PUNICODE_STRING
# Desc  : ����̹� ������
*/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriver, IN PUNICODE_STRING pRegPath)
{
	UNREFERENCED_PARAMETER(pRegPath);
	UNREFERENCED_PARAMETER(pDriver);
	NTSTATUS ret = STATUS_SUCCESS;
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] Load Driver\n");
	pDriver->DriverUnload = UnloadDriver;
	ret = ObRegExample();
	if (ret==STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] Success Registeration\n");
	}
	else
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] Failed Registration %IX\n",ret);
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
	if (hRegistration)
	{
		ObUnRegisterCallbacks(hRegistration);
	}
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "[+] Unload Driver\n");
}