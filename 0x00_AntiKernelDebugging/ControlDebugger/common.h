#pragma once
#include <ntifs.h>

/*///////////////////////////////////////////////////
# File : common.h
# Desc : 모든 함수와 구조체, 전역변수 등 선언 및 정의
*////////////////////////////////////////////////////


#define DeviceName L"\\Device\\CONTROL_DEBUGGER"
#define DEBUGGER_ENABLE 0x01
#define DEBUGGER_DISABLE 0x02
#define KDCOM_HOOKING 0x03
#define OVERWRITE_CALLBACKS 0x04
#define RESTORE_CALLBACKS 0x05
#define OVERWRITE_DEBUGPORT 0x06

//============================================//
//================ Structure =================//
//============================================//

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation, // q: SYSTEM_BASIC_INFORMATION
	SystemProcessorInformation, // q: SYSTEM_PROCESSOR_INFORMATION
	SystemPerformanceInformation, // q: SYSTEM_PERFORMANCE_INFORMATION
	SystemTimeOfDayInformation, // q: SYSTEM_TIMEOFDAY_INFORMATION
	SystemPathInformation, // not implemented
	SystemProcessInformation, // q: SYSTEM_PROCESS_INFORMATION
	SystemCallCountInformation, // q: SYSTEM_CALL_COUNT_INFORMATION
	SystemDeviceInformation, // q: SYSTEM_DEVICE_INFORMATION
	SystemProcessorPerformanceInformation, // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
	SystemFlagsInformation, // q: SYSTEM_FLAGS_INFORMATION
	SystemCallTimeInformation, // not implemented // SYSTEM_CALL_TIME_INFORMATION // 10
	SystemModuleInformation, // q: RTL_PROCESS_MODULES
	
} SYSTEM_INFORMATION_CLASS;
typedef struct _SYSTEM_MODULE_ENTRY
{
	HANDLE Section;				//0x0000(0x0008)
	PVOID MappedBase;			//0x0008(0x0008)
	PVOID ImageBase;			//0x0010(0x0008)
	ULONG ImageSize;			//0x0018(0x0004)
	ULONG Flags;				//0x001C(0x0004)
	USHORT LoadOrderIndex;		//0x0020(0x0002)
	USHORT InitOrderIndex;		//0x0022(0x0002)
	USHORT LoadCount;			//0x0024(0x0002)
	USHORT OffsetToFileName;	//0x0026(0x0002)
	UCHAR FullPathName[256];	//0x0028(0x0100)
} SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;
typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG               Count;
	SYSTEM_MODULE_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;
typedef struct _CALLBACK_ENTRY
{
	INT16							Version;
	unsigned char					unknown[6];
	POB_OPERATION_REGISTRATION		RegistrationContext;
	UNICODE_STRING					Altitude;
} CALLBACK_ENTRY, * PCALLBACK_ENTRY;
typedef struct _CALLBACK_ENTRY_ITEM
{
	LIST_ENTRY						EntryItemList;
	OB_OPERATION					Operations1;
	OB_OPERATION					Operations2;
	PCALLBACK_ENTRY					CallbackEntry;
	POBJECT_TYPE					ObjectType;
	POB_PRE_OPERATION_CALLBACK		PreOperation;
	POB_POST_OPERATION_CALLBACK		PostOperation;
} CALLBACK_ENTRY_ITEM, * PCALLBACK_ENTRY_ITEM;
typedef struct _OBJECT_TYPE
{
	LIST_ENTRY                 TypeList;
	UNICODE_STRING             Name;
	PVOID                      DefaultObject;
	ULONG                      Index;
	ULONG                      TotalNumberOfObjects;
	ULONG                      TotalNumberOfHandles;
	ULONG                      HighWaterNumberOfObjects;
	ULONG                      HighWaterNumberOfHandles;
	unsigned char			   TypeInfo[0x78];
	EX_PUSH_LOCK               TypeLock;
	ULONG                      Key;
	LIST_ENTRY                 CallbackList;
} OBJECT_TYPE, * POBJECT_TYPE;

typedef struct _IMPORT_OFFSET
{
	int			UniqueProcessid_off;
	int			ActiveProcessLinks_off;
	int			ImageFileName_off;
	int			PEB_off;
	int         DebugPort_off;
}IMPORT_OFFSET;



//============================================//
//============= Glboal Variable ==============//
//============================================//

PVOID RegistrationHandle = NULL;
PVOID pBackupCallback = NULL;
PDEVICE_OBJECT pDevice = NULL;

UNICODE_STRING SymbolickLink = { 0, };
UNICODE_STRING NtQuerySystemInformationString = { 0, };
IMPORT_OFFSET iOffset = { 0, };
SYSTEM_MODULE_ENTRY TargetModule = { 0, };

const char szSystem[] = "System";
const wchar_t szNtQueryInformationProcess[] = L"NtQueryInformationProcess";

unsigned char bPatchBytes[5] = { 0xC6,0x01,0x01,0x90,0x90 };	// KdDebuggerPresent




//============================================//
//=========== Undocumented API ===============//
//============================================//

typedef NTSTATUS(*NtQuerySystemInformation_t)(
	_In_	SYSTEM_INFORMATION_CLASS	SystemInformationClass,
	_Out_	PVOID						SystemInformation,
	_In_	ULONG						SystemInformationLength,
	_Out_	PULONG						ReturnLength OPTIONAL
	);

typedef NTSTATUS(*NtQueryInformationProcess_t)(
	IN    HANDLE              ProcessHandle,
	IN    PROCESSINFOCLASS    ProcessInformationClass,
	OUT   PVOID               ProcessInformation,
	IN    ULONG               ProcessInformationLength,
	OUT   PULONG              ReturnLength
	);


//============================================//
//======= DriverEntry & Unload Routine =======//
//============================================//

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriver, IN PUNICODE_STRING pRegPath);
VOID UnloadDriver(IN PDRIVER_OBJECT pDriver);


//============================================//
//========== User-defined Function  ==========//
//============================================//

VOID Dummy();

NTSTATUS GetModuleInformation(const char* szModuleName);
BOOLEAN GetOffset(PEPROCESS Process);
BOOLEAN GetPebOffset();

NTSTATUS ControlDebugger(PDEVICE_OBJECT pDevice, PIRP pIrp);
NTSTATUS KdDebuggerControl(int mode);
NTSTATUS Hook_KdReceivePacket();
NTSTATUS OverWriteCallbacks();
NTSTATUS OverWriteDebugPort(PIRP pIrp);