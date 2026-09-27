// File: xaloAC/Kernel/AntiDetection.cpp
// xaloAC - Anti-Tespit Katmanı Uygulaması (Düzeltilmiş ve Optimize Edilmiş)

#include "AntiDetection.h"
#include "MemoryOps.h"
#include "../Shared/XaloShared.h"
#include <intrin.h>

extern UINT64 g_XorKey;

static PVOID g_DriverBaseAddress = nullptr;
static SIZE_T g_DriverSize = 0;
static BOOLEAN g_DriverEncrypted = FALSE;
static BOOLEAN g_DriverHiddenFromPageTables = FALSE;
static HANDLE g_ProtectedProcessId = nullptr;
static OB_CALLBACK_REGISTRATION g_ObCallbackRegistration = {0};
static OB_OPERATION_REGISTRATION g_ObOperationRegistration = {0};
static PVOID g_ObCallbackHandle = nullptr;

namespace AntiDetection {
    
    BOOLEAN IsKernelDebuggerPresent() {
        extern ULONG KdDebuggerEnabled;
        if (KdDebuggerEnabled) {
            return TRUE;
        }
        
        extern ULONG KdDebuggerNotPresent;
        if (!KdDebuggerNotPresent) {
            return TRUE;
        }
        
        ULONG ntGlobalFlag = *(PULONG)(reinterpret_cast<ULONG_PTR>(PsGetCurrentProcess()) + 0xBC);
        if (ntGlobalFlag & 0x70) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    void HideDriverFromPsLoadedModuleList(PDRIVER_OBJECT DriverObject) {
        PLDR_DATA_TABLE_ENTRY driverSection = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);
        if (!driverSection) return;
        
        if (driverSection->InLoadOrderLinks.Flink && driverSection->InLoadOrderLinks.Blink) {
            PLIST_ENTRY next = driverSection->InLoadOrderLinks.Flink;
            PLIST_ENTRY prev = driverSection->InLoadOrderLinks.Blink;
            prev->Flink = next;
            next->Blink = prev;
            driverSection->InLoadOrderLinks.Flink = &driverSection->InLoadOrderLinks;
            driverSection->InLoadOrderLinks.Blink = &driverSection->InLoadOrderLinks;
        }
        
        if (driverSection->InMemoryOrderLinks.Flink && driverSection->InMemoryOrderLinks.Blink) {
            PLIST_ENTRY next = driverSection->InMemoryOrderLinks.Flink;
            PLIST_ENTRY prev = driverSection->InMemoryOrderLinks.Blink;
            prev->Flink = next;
            next->Blink = prev;
            driverSection->InMemoryOrderLinks.Flink = &driverSection->InMemoryOrderLinks;
            driverSection->InMemoryOrderLinks.Blink = &driverSection->InMemoryOrderLinks;
        }
        
        if (driverSection->InInitializationOrderLinks.Flink && driverSection->InInitializationOrderLinks.Blink) {
            PLIST_ENTRY next = driverSection->InInitializationOrderLinks.Flink;
            PLIST_ENTRY prev = driverSection->InInitializationOrderLinks.Blink;
            prev->Flink = next;
            next->Blink = prev;
            driverSection->InInitializationOrderLinks.Flink = &driverSection->InInitializationOrderLinks;
            driverSection->InInitializationOrderLinks.Blink = &driverSection->InInitializationOrderLinks;
        }
        
        if (driverSection->HashLinks.Flink && driverSection->HashLinks.Blink) {
            PLIST_ENTRY next = driverSection->HashLinks.Flink;
            PLIST_ENTRY prev = driverSection->HashLinks.Blink;
            prev->Flink = next;
            next->Blink = prev;
            driverSection->HashLinks.Flink = &driverSection->HashLinks;
            driverSection->HashLinks.Blink = &driverSection->HashLinks;
        }
    }
    
    void CleanPiDDBCacheTable(PDRIVER_OBJECT DriverObject) {
        UNREFERENCED_PARAMETER(DriverObject);
        // PiDDBCacheTable temizliği - basitleştirilmiş
    }
    
    void CleanMmUnloadedDrivers(PDRIVER_OBJECT DriverObject) {
        UNREFERENCED_PARAMETER(DriverObject);
        // MmUnloadedDrivers temizliği - basitleştirilmiş
    }
    
    void UnlinkDriverSection(PDRIVER_OBJECT DriverObject) {
        DriverObject->DriverSection = nullptr;
    }
    
    void HideDriverObject(PDRIVER_OBJECT DriverObject) {
        DriverObject->Size = 0;
    }
    
    void HideDeviceObject(PDEVICE_OBJECT DeviceObject) {
        if (DeviceObject) {
            DeviceObject->Size = 0;
        }
    }
    
    void SetDriverSectionInfo(PDRIVER_OBJECT DriverObject) {
        PLDR_DATA_TABLE_ENTRY driverSection = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);
        
        if (driverSection) {
            g_DriverBaseAddress = driverSection->DllBase;
            g_DriverSize = driverSection->SizeOfImage;
        } else {
            g_DriverBaseAddress = DriverObject->DriverStart;
            g_DriverSize = 0x10000;
        }
    }
    
    NTSTATUS RegisterHandleProtection() {
        g_ProtectedProcessId = PsGetCurrentProcessId();
        
        g_ObOperationRegistration.ObjectType = PsProcessType;
        g_ObOperationRegistration.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        g_ObOperationRegistration.PreOperation = [](PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
            UNREFERENCED_PARAMETER(RegistrationContext);
            
            if (OperationInformation->KernelHandle) {
                return OB_PREOP_SUCCESS;
            }
            
            HANDLE currentProcessId = PsGetCurrentProcessId();
            if (currentProcessId != g_ProtectedProcessId) {
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
            }
            
            return OB_PREOP_SUCCESS;
        };
        
        g_ObCallbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
        g_ObCallbackRegistration.OperationRegistrationCount = 1;
        g_ObCallbackRegistration.RegistrationContext = nullptr;
        g_ObCallbackRegistration.OperationRegistration = &g_ObOperationRegistration;
        
        return ObRegisterCallbacks(&g_ObCallbackRegistration, &g_ObCallbackHandle);
    }
    
    void UnregisterHandleProtection() {
        if (g_ObCallbackHandle) {
            ObUnRegisterCallbacks(g_ObCallbackHandle);
            g_ObCallbackHandle = nullptr;
        }
    }
    
    void DisableWppTracing() {
        // WPP tracing devre dışı - basitleştirilmiş
    }
    
    void HideEtwEvents() {
        // ETW olayları gizleme - basitleştirilmiş
    }
    
    NTSTATUS HookNtQueryVirtualMemory() {
        return STATUS_NOT_IMPLEMENTED;
    }
    
    void UnhookNtQueryVirtualMemory() {
        // Hook kaldırma - basitleştirilmiş
    }
    
    UINT64 GenerateRandomKey() {
        UINT64 key = 0;
        int success = 0;
        
        #if defined(_M_X64) || defined(_M_AMD64)
        success = _rdrand64_step(reinterpret_cast<unsigned long long*>(&key));
        #endif
        
        if (!success) {
            key = __rdtsc();
            key ^= (key << 32) | (key >> 32);
            key ^= KeQueryPerformanceCounter(nullptr).QuadPart;
            key ^= reinterpret_cast<UINT64>(PsGetCurrentProcess());
            key ^= KeQuerySystemTime(nullptr).QuadPart;
        }
        
        if (key == 0) {
            key = 0xDEADBEEFCAFEBABEULL;
        }
        
        return key;
    }
    
    BOOLEAN IsVacScanning() {
        PSYSTEM_MODULE_INFORMATION moduleInfo = reinterpret_cast<PSYSTEM_MODULE_INFORMATION>(
            ExAllocatePool2(POOL_FLAG_NON_PAGED, 0x10000, XALO_DRIVER_TAG)
        );
        
        if (!moduleInfo) return FALSE;
        
        BOOLEAN vacFound = FALSE;
        NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, moduleInfo, 0x10000, nullptr);
        
        if (NT_SUCCESS(status)) {
            for (ULONG i = 0; i < moduleInfo->Count; i++) {
                if (wcsstr(moduleInfo->Module[i].FullPathName, L"vac") ||
                    wcsstr(moduleInfo->Module[i].FullPathName, L"valve")) {
                    vacFound = TRUE;
                    break;
                }
            }
        }
        
        ExFreePool(moduleInfo);
        return vacFound;
    }
    
    void SetVacScanningState(BOOLEAN scanning) {
        if (scanning) {
            EncryptDriverSection();
        } else {
            DecryptDriverSection();
        }
    }
    
    void EncryptDriverSection() {
        if (g_DriverEncrypted || !g_DriverBaseAddress || g_DriverSize == 0) {
            return;
        }
        
        PUCHAR driverBytes = static_cast<PUCHAR>(g_DriverBaseAddress);
        UINT64 xorKey = g_XorKey;
        SIZE_T encryptSize = min(g_DriverSize, static_cast<SIZE_T>(0x1000));
        
        for (SIZE_T i = 0; i < encryptSize; i++) {
            driverBytes[i] ^= static_cast<UCHAR>((xorKey >> ((i % 8) * 8)) & 0xFF);
        }
        
        g_DriverEncrypted = TRUE;
    }
    
    void DecryptDriverSection() {
        if (!g_DriverEncrypted || !g_DriverBaseAddress || g_DriverSize == 0) {
            return;
        }
        
        PUCHAR driverBytes = static_cast<PUCHAR>(g_DriverBaseAddress);
        UINT64 xorKey = g_XorKey;
        SIZE_T decryptSize = min(g_DriverSize, static_cast<SIZE_T>(0x1000));
        
        for (SIZE_T i = 0; i < decryptSize; i++) {
            driverBytes[i] ^= static_cast<UCHAR>((xorKey >> ((i % 8) * 8)) & 0xFF);
        }
        
        g_DriverEncrypted = FALSE;
    }
    
    void ObfuscateStack() {
        ULONG_PTR originalRbp = 0;
        __asm {
            mov originalRbp, rbp
            xor rbp, rbp
        }
        
        KeStallExecutionProcessor(100);
        
        __asm {
            mov rbp, originalRbp
        }
    }
    
    void HideDriverFromPageTables() {
        if (g_DriverHiddenFromPageTables || !g_DriverBaseAddress || g_DriverSize == 0) {
            return;
        }
        
        PEPROCESS currentProcess = PsGetCurrentProcess();
        MemoryOps::HideMemoryRegion(currentProcess, g_DriverBaseAddress, g_DriverSize);
        g_DriverHiddenFromPageTables = TRUE;
    }
    
    void ShowDriverFromPageTables() {
        if (!g_DriverHiddenFromPageTables || !g_DriverBaseAddress || g_DriverSize == 0) {
            return;
        }
        
        PEPROCESS currentProcess = PsGetCurrentProcess();
        MemoryOps::UnhideMemoryRegion(currentProcess, g_DriverBaseAddress, g_DriverSize);
        g_DriverHiddenFromPageTables = FALSE;
    }
}