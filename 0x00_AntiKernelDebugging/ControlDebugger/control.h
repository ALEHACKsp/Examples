#pragma once
#include "customapi.h"
#include <stdlib.h>

/*//////////////////////////////////////////////
# File : control.h
# Desc : 디버거 제어와 관련된 함수 정의
*///////////////////////////////////////////////


/*
# Name  : KdDebuggerControl
# Param : int
# Desc  : KdDebuggerEnabled 변수 제어
# Case  : 0x1, 0x2
*/
NTSTATUS KdDebuggerControl(int mode)
{
	if (mode == DEBUGGER_ENABLE)
	{
		*KdDebuggerEnabled = TRUE;		
	}
	else if (mode == DEBUGGER_DISABLE)
	{
		*KdDebuggerEnabled = FALSE;
	}
	return STATUS_SUCCESS;
}

/*
# Name  : Hook_KdReceivePacket
# Param : x
# Desc  : KdDebuggerNotPresent 변조용
# Case  : 0x3
*/
NTSTATUS Hook_KdReceivePacket()
{
	NTSTATUS Status = STATUS_SUCCESS;
	PVOID KdReceivePacket = NULL;
	Status = GetModuleInformation("\\SystemRoot\\System32\\kdcom.dll");
	if (Status != STATUS_SUCCESS)
	{
		return STATUS_INVALID_ADDRESS;
	}
	else
	{
		KdReceivePacket = (DWORD64)TargetModule.ImageBase + 0x1861;	// KdReceivePacket+4a1 ( Write KdDebuggerNotPresent )
		memcpy(KdReceivePacket, bPatchBytes, 5);
		return STATUS_SUCCESS;
	}
}

/*
# Name  : OverWriteCallbacks
# Param : int
# Desc  : 콜백 루틴을 더미 함수로 변조
# Case  : 0x4, 0x5
*/
NTSTATUS OverWriteCallbacks(int mode)
{
	POBJECT_TYPE* obType = PsProcessType;
	PCALLBACK_ENTRY_ITEM CallbackEntry = NULL;

	if (mode == OVERWRITE_CALLBACKS)
	{
		CallbackEntry = (*obType)->CallbackList.Flink;

		if (MmIsAddressValid(CallbackEntry))
		{
			pBackupCallback = CallbackEntry->PreOperation;
			CallbackEntry->PreOperation = &Dummy;
			return STATUS_SUCCESS;
		}
	}
	else if (mode == RESTORE_CALLBACKS)
	{
		CallbackEntry = (*obType)->CallbackList.Flink;

		if (MmIsAddressValid(CallbackEntry))
		{
			if (pBackupCallback)
			{
				CallbackEntry->PreOperation = pBackupCallback;
				
				return STATUS_SUCCESS;
			}
			else
			{
				return STATUS_ACCESS_DENIED;
			}
		}
	}
	
	return STATUS_ACCESS_DENIED;
}

/*
# Name  : OverWriteDebugPort
# Param : PIRP
# Desc  : 프로세스 디버그 포트 제어
# Case  : 0x6
*/
NTSTATUS OverWriteDebugPort(PIRP pIrp)
{
	int targetPID = atoi(pIrp->AssociatedIrp.SystemBuffer);
	PEPROCESS Process = NULL;
	PVOID pDebugPort = NULL;
	
	if (PsLookupProcessByProcessId(targetPID, &Process) == STATUS_SUCCESS)
	{
		pDebugPort = (void*)((DWORD64)Process + iOffset.DebugPort_off);
		if (MmIsAddressValid(pDebugPort))
		{
			memset(pDebugPort, 0, 8);
		}
	}

	return STATUS_SUCCESS;
}


NTSTATUS ControlDebugger(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	PIO_STACK_LOCATION pStack = NULL;
	ULONG ControlCode = 0;
	NTSTATUS Status = STATUS_SUCCESS;
	
	pStack = IoGetCurrentIrpStackLocation(pIrp);
	ControlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
	if (ControlCode)
	{
		switch (ControlCode)
		{
		case DEBUGGER_ENABLE:
			Status = KdDebuggerControl(DEBUGGER_ENABLE);
			if (Status == STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Enable Debugger\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Control Debugger\n");
			}
			break;

		case DEBUGGER_DISABLE:
			Status = KdDebuggerControl(DEBUGGER_DISABLE);
			if (Status == STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Disable Debugger\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Control Debugger\n");
			}
			break;

		case KDCOM_HOOKING:
			Status = Hook_KdReceivePacket();
			if (Status == STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Patch Complete\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Patch\n");
			}
			break;

		case OVERWRITE_CALLBACKS:
			Status = OverWriteCallbacks(OVERWRITE_CALLBACKS);
			if (Status == STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Overwrite Callbacks\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Overwrite\n");
			}
			break;

		case RESTORE_CALLBACKS:
			Status = OverWriteCallbacks(RESTORE_CALLBACKS);
			if (Status == STATUS_SUCCESS)
			{
				
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] Restore Callbacks\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Restore\n");
			}
			break;

		case OVERWRITE_DEBUGPORT:
			Status = OverWriteDebugPort(pIrp);
			if (Status == STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_INFO_LEVEL, "[INFO] DebugPort Overwrite\n");
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, DPFLTR_ERROR_LEVEL, "[ERROR] Can't Overwrite Debugport\n");
			}
			break;
		}
	}
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}