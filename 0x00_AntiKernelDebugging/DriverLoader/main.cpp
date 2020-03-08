#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

#pragma warning(disable:4996)

#define DRIVERNAME _T("\\ControlDebugger.sys")
#define SERVICENAME _T("ControlDebugger")
#define ORDERGROUP _T("Shh0ya")

BOOL DriverLoader()
{
	TCHAR DriverPath[MAX_PATH] = { 0, };
	TCHAR currPath[MAX_PATH] = { 0, };

	lstrcpyW(DriverPath, _T("\\??\\"));
	GetCurrentDirectory(MAX_PATH, currPath);
	lstrcatW(DriverPath, currPath);
	lstrcatW(DriverPath, DRIVERNAME);

	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE hService = CreateService(
		hScm, SERVICENAME, SERVICENAME, SC_MANAGER_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, DriverPath, ORDERGROUP, NULL, NULL, NULL, NULL);

	if (!hService)
	{
		if (GetLastError() != 0x431)
		{
			printf("[!] Error Code : 0x%X(%d)\n", GetLastError(), GetLastError());
			CloseHandle(hScm);
			system("pause");
			return FALSE;
		}
		else
		{
			hService = OpenService(hScm, SERVICENAME, SC_MANAGER_ALL_ACCESS);
			if (!hService)
			{
				printf("[!] Open Service Error : 0x%X(%d)\n", GetLastError(), GetLastError());
				system("pause");
				CloseHandle(hScm);
				return FALSE;
			}
		}
	}

	if (!StartService(hService, 0, NULL))
	{
		printf("[!] Service Start Error : 0x%X(%d)\n", GetLastError(), GetLastError());
		system("pause");
		DeleteService(hService);
		CloseHandle(hService);
		CloseHandle(hScm);
		return FALSE;
	}
	CloseHandle(hService);
	CloseHandle(hScm);
	return TRUE;
}

void StopService()
{
	SERVICE_STATUS Status;
	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE hService = OpenService(hScm, SERVICENAME, SERVICE_STOP);
	ControlService(hService, SERVICE_CONTROL_STOP, &Status);

	CloseHandle(hScm);
	CloseHandle(hService);

	return;
}

void SendControl(int mode)
{
	printf("Success Code, %d\n", mode);

	HANDLE deviceHandle;
	TCHAR linkName[] = _T("\\\\.\\ControlDebugger");
	DWORD dwRet = NULL;
	deviceHandle = CreateFile(linkName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (deviceHandle == INVALID_HANDLE_VALUE)
	{
		printf("[!] Invalid handle error\n");
		return;
	}

	if (!DeviceIoControl(deviceHandle, mode, 0, 0, 0, 0, &dwRet, 0))
	{
		printf("[+] Send Control Code\n");
		CloseHandle(deviceHandle);
		return;
	}

	CloseHandle(deviceHandle);
	return;
}
#define KDCOM_HOOKING 0x03
#define OVERWRITE_CALLBACKS 0x04
#define RESTORE_CALLBACKS 0x05
#define OVERWRITE_DEBUGPORT 0x06
int main()
{
	printf("[#] Driver Loader\n");
	printf("[#] Load Driver...\n");
	if (DriverLoader())
	{
		printf("[#] Driver Load Success!!!\n");
		Sleep(500);
		system("cls");
		int sel = 0;
		while (1)
		{
			printf("[#] Select\n\n");
			printf("[1] Debugger Enable\n");
			printf("[2] Debugger Disable\n");
			printf("[3] Kdcom Hooking\n");
			printf("[4] Overwrite Callbacks\n");
			printf("[5] Restore Callbacks\n");
			printf("[6] Overwrite DebugPort\n");
			printf("[7] Unload Driver\n");

			printf(": ");
			scanf("%d", &sel);
			system("cls");

			if (sel == 0x7)
			{
				StopService();
				break;
			}

			if (sel)
			{
				SendControl(sel);
			}
		}
		return 0;
	}

	else
	{
		printf("[!] Driver Load Failed...\n");
		return 0;
	}
}