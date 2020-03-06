#pragma once
#include "offset.h"

/*//////////////////////////////////////////////
# File : callbacks.h
# Desc : �ݹ� ��ƾ�� ���� ���ǿ� ���� �Լ� ����
*///////////////////////////////////////////////


/*
# Name  : LoadImageNotifyRoutine
# Param : PUNICODE_STRING, HANDLE, PIMAGE_INFO
# Desc  : KdDebuggerEnabled �� KdDebuggerNotPresent Ŀ�� ���� ������ Ȱ��
*/
VOID LoadImageNotifyRoutine(
	IN PUNICODE_STRING FullImageName, 
	IN HANDLE ProcessId, 
	IN PIMAGE_INFO ImageInfo)
{
	PEPROCESS *Process = NULL;
	char szProcName[16] = { 0, };


	if (!ImageInfo->SystemModeImage)
	{
		if (PsLookupProcessByProcessId(ProcessId, &Process) == STATUS_SUCCESS)
		{
			strcpy_s(szProcName, 16, (PVOID*)((PCHAR)Process + iOffset.ImageFileName_off));
			if (!_strnicmp(szProcName, szTarget, 16))
			{
				if (*KdDebuggerNotPresent==FALSE)
				{
					DbgPrintEx(DPFLTR_ACPI_ID, 1, "[WARN] Debugger Present\n");
				}
				else
				{
					if (*KdDebuggerEnabled)
					{
						DbgPrintEx(DPFLTR_ACPI_ID, 1, "[WARN] Kernel Debugger Enabled \n");
					}
				}
			}
		}
	}
}

/*
# Name  : PreCallback
# Param : PVOID, POB_PRE_OPERATION_INFORMATION
# Desc  : PsGetProcessDebugPort �� �̿��Ͽ� ������� ����� ����
*/
OB_PREOP_CALLBACK_STATUS PreCallback(
	PVOID RegistrationContext, 
	POB_PRE_OPERATION_INFORMATION pOperationInformation
)
{
	UNREFERENCED_PARAMETER(RegistrationContext);

	char szProcName[16] = { 0, };
	strcpy_s(szProcName, 16, ((DWORD64)pOperationInformation->Object + iOffset.ImageFileName_off));
	if (!_strnicmp(szProcName, szTarget, 16))
	{
		if (PsGetProcessDebugPort(pOperationInformation->Object))
		{
			if (!bOnOff)
			{
				bOnOff = TRUE;
				TerminateProcess(PsGetProcessId(pOperationInformation->Object));
				bOnOff = FALSE;
			}
		}
	}
}

/*
# Name  : PostCallback
# Param : PVOID, POB_POST_OPERATION_INFORMATION
# Desc  : ������� ���� �� ����
*/
VOID PostCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION pOperationInformation)
{
	UNREFERENCED_PARAMETER(RegistrationContext);

	// your code

}

/*
# Name  : TerminateProcess
# Param : HANDLE
# Desc  : ���μ��� ���� ���� �� ���
*/
VOID TerminateProcess(IN HANDLE pid)
{
	HANDLE hProcess = NULL;
	OBJECT_ATTRIBUTES obAttr = { 0, };
	CLIENT_ID cid = { 0, };

	obAttr.Length = sizeof(obAttr);
	obAttr.Attributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
	cid.UniqueProcess = pid;

	if (ZwOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &obAttr, &cid) 
		== STATUS_SUCCESS)	// Get process handle
	{
		if (ZwTerminateProcess(hProcess, STATUS_ACCESS_DENIED) 
			== STATUS_SUCCESS)	// Terminate process
		{
			DbgPrintEx(DPFLTR_ACPI_ID, 3,
				"[INFO] Success terminate process\n");
		}
		else
		{
			DbgPrintEx(DPFLTR_ACPI_ID, 0,
				"[ERR] Failed terminate process\n");
		}
	}
	else
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0,
			"[ERR] Failed open process\n");
	}
}

/*
# Name  : ObCallbackReg
# Param : x
# Desc  : ObRegisterCallbacks ȣ��
*/
NTSTATUS ObCallbackReg()
{
	OB_CALLBACK_REGISTRATION obRegistration = { 0, };
	OB_OPERATION_REGISTRATION opRegistration = { 0, };

	obRegistration.Version = ObGetFilterVersion();	// Get version
	obRegistration.OperationRegistrationCount = 1;	// OB_OPERATION_REGISTRATION count, opRegistration[2] �� ��� 2
	RtlInitUnicodeString(&obRegistration.Altitude, L"300000");	// ������ Altitude ����
	obRegistration.RegistrationContext = NULL;

	opRegistration.ObjectType = PsProcessType;
	opRegistration.Operations = OB_OPERATION_HANDLE_CREATE;	// Create �Ǵ� Open �� ����
	opRegistration.PreOperation = PreCallback;	// PreOperation ���
	opRegistration.PostOperation = PostCallback;	// PostOperation ���

	obRegistration.OperationRegistration = &opRegistration;	// OperationRegistration ���

	return ObRegisterCallbacks(&obRegistration, &hRegistration);
}