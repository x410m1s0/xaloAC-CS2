// File: xaloAC/Kernel/DriverMain.cpp
// xaloAC - Kernel Sürücüsü Ana Giriş Noktası (Düzeltilmiş ve Optimize Edilmiş)

#include <ntddk.h>
#include <ntstrsafe.h>
#include "MemoryOps.h"
#include "AntiDetection.h"
#include "Communication.h"
#include "Hypervisor.h"
#include "PEHider.h"
#include "../Shared/XaloShared.h"

PDEVICE_OBJECT g_DeviceObject = nullptr;
UNICODE_STRING g_DeviceName;
UNICODE_STRING g_SymbolicLink;
PVOID g_SharedMemory = nullptr;
SIZE_T g_SharedMemorySize = XALO_SHARED_MEMORY_SIZE;
UINT64 g_XorKey = 0;
volatile BOOLEAN g_DriverLoaded = FALSE;

NTSTATUS XaloCreateDevice(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\XaloACDriver");
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\XaloACDriver");
    
    NTSTATUS status = IoCreateDevice(
        DriverObject, 0, &deviceName,
        FILE_DEVICE_UNKNOWN, 0, FALSE, &g_DeviceObject
    );
    
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = nullptr;
        return status;
    }
    
    g_DeviceName = deviceName;
    g_SymbolicLink = symbolicLink;
    
    AntiDetection::HideDeviceObject(g_DeviceObject);
    
    return STATUS_SUCCESS;
}

NTSTATUS XaloDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    PVOID outputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG outputLength = stack->Parameters.DeviceIoControl.OutputBufferLength;
    
    NTSTATUS status = STATUS_SUCCESS;
    ULONG bytesReturned = 0;
    
    switch (controlCode) {
        case XALO_IOCTL_GET_SHARED_MEMORY: {
            if (outputBuffer && outputLength >= sizeof(ULONG_PTR)) {
                *(PULONG_PTR)outputBuffer = reinterpret_cast<ULONG_PTR>(g_SharedMemory);
                bytesReturned = sizeof(ULONG_PTR);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }
        case XALO_IOCTL_GET_XOR_KEY: {
            if (outputBuffer && outputLength >= sizeof(UINT64)) {
                *(PUINT64)outputBuffer = g_XorKey;
                bytesReturned = sizeof(UINT64);
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }
        case XALO_IOCTL_GET_MODULE_BASE: {
            // Modül adı inputBuffer'da, sonuç outputBuffer'da
            if (inputBuffer && outputBuffer) {
                wchar_t* moduleName = static_cast<wchar_t*>(inputBuffer);
                // ProcessId inputBuffer'ın sonunda
                HANDLE processId = *reinterpret_cast<HANDLE*>(
                    static_cast<PUCHAR>(inputBuffer) + (wcslen(moduleName) + 1) * sizeof(wchar_t)
                );
                
                PEPROCESS process = nullptr;
                status = MemoryOps::GetProcessById(processId, &process);
                
                if (NT_SUCCESS(status)) {
                    PVOID baseAddress = nullptr;
                    SIZE_T moduleSize = 0;
                    status = MemoryOps::GetModuleBaseAddress(process, moduleName, &baseAddress, &moduleSize);
                    
                    if (NT_SUCCESS(status)) {
                        struct ModuleResult {
                            ULONG_PTR BaseAddress;
                            SIZE_T Size;
                        } result;
                        
                        result.BaseAddress = reinterpret_cast<ULONG_PTR>(baseAddress);
                        result.Size = moduleSize;
                        
                        RtlCopyMemory(outputBuffer, &result, sizeof(result));
                        bytesReturned = sizeof(result);
                    }
                    
                    ObDereferenceObject(process);
                }
            }
            break;
        }
        case XALO_IOCTL_READ_MEMORY: {
            // Bellek okuma isteği
            if (inputBuffer && outputBuffer) {
                struct ReadRequest {
                    HANDLE ProcessId;
                    ULONG_PTR Address;
                    SIZE_T Size;
                };
                
                ReadRequest* request = static_cast<ReadRequest*>(inputBuffer);
                
                PEPROCESS process = nullptr;
                status = MemoryOps::GetProcessById(request->ProcessId, &process);
                
                if (NT_SUCCESS(status)) {
                    status = MemoryOps::ReadVirtualMemory(
                        process,
                        reinterpret_cast<PVOID>(request->Address),
                        outputBuffer,
                        min(request->Size, static_cast<SIZE_T>(outputLength))
                    );
                    
                    if (NT_SUCCESS(status)) {
                        bytesReturned = static_cast<ULONG>(min(request->Size, static_cast<SIZE_T>(outputLength)));
                    }
                    
                    ObDereferenceObject(process);
                }
            }
            break;
        }
        case XALO_IOCTL_WRITE_MEMORY: {
            if (inputBuffer) {
                struct WriteRequest {
                    HANDLE ProcessId;
                    ULONG_PTR Address;
                    SIZE_T Size;
                    BYTE Data[512];
                };
                
                WriteRequest* request = static_cast<WriteRequest*>(inputBuffer);
                
                PEPROCESS process = nullptr;
                status = MemoryOps::GetProcessById(request->ProcessId, &process);
                
                if (NT_SUCCESS(status)) {
                    status = MemoryOps::WriteVirtualMemory(
                        process,
                        reinterpret_cast<PVOID>(request->Address),
                        request->Data,
                        min(request->Size, static_cast<SIZE_T>(512))
                    );
                    
                    ObDereferenceObject(process);
                }
            }
            break;
        }
        case XALO_IOCTL_GET_PROCESS_ID: {
            if (inputBuffer && outputBuffer) {
                wchar_t* processName = static_cast<wchar_t*>(inputBuffer);
                
                ULONG bufferSize = 0;
                ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &bufferSize);
                
                if (bufferSize > 0) {
                    PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, XALO_DRIVER_TAG);
                    
                    if (buffer) {
                        status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, nullptr);
                        
                        if (NT_SUCCESS(status)) {
                            PSYSTEM_PROCESS_INFORMATION processInfo = static_cast<PSYSTEM_PROCESS_INFORMATION>(buffer);
                            
                            while (true) {
                                if (processInfo->ImageName.Buffer && wcsstr(processInfo->ImageName.Buffer, processName)) {
                                    *(PHANDLE)outputBuffer = processInfo->UniqueProcessId;
                                    bytesReturned = sizeof(HANDLE);
                                    status = STATUS_SUCCESS;
                                    break;
                                }
                                
                                if (processInfo->NextEntryOffset == 0) {
                                    status = STATUS_NOT_FOUND;
                                    break;
                                }
                                
                                processInfo = reinterpret_cast<PSYSTEM_PROCESS_INFORMATION>(
                                    reinterpret_cast<ULONG_PTR>(processInfo) + processInfo->NextEntryOffset
                                );
                            }
                        }
                        
                        ExFreePool(buffer);
                    }
                }
            }
            break;
        }
        case XALO_IOCTL_SHUTDOWN: {
            Communication::CleanupSharedMemory(g_SharedMemory, g_SharedMemorySize);
            g_DriverLoaded = FALSE;
            bytesReturned = 0;
            break;
        }
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = bytesReturned;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}

NTSTATUS XaloCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    
    NTSTATUS status;
    
    if (AntiDetection::IsKernelDebuggerPresent()) {
        return STATUS_DEBUG_ATTACH_FAILED;
    }
    
    AntiDetection::DisableWppTracing();
    AntiDetection::HideEtwEvents();
    AntiDetection::HideDriverFromPsLoadedModuleList(DriverObject);
    AntiDetection::CleanPiDDBCacheTable(DriverObject);
    AntiDetection::CleanMmUnloadedDrivers(DriverObject);
    AntiDetection::UnlinkDriverSection(DriverObject);
    AntiDetection::HideDriverObject(DriverObject);
    AntiDetection::RegisterHandleProtection();
    
    DriverObject->MajorFunction[IRP_MJ_CREATE] = XaloCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = XaloCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = XaloDeviceControl;
    
    status = XaloCreateDevice(DriverObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    status = Communication::InitializeSharedMemory(&g_SharedMemory, g_SharedMemorySize);
    if (!NT_SUCCESS(status)) {
        IoDeleteSymbolicLink(&g_SymbolicLink);
        IoDeleteDevice(g_DeviceObject);
        return status;
    }
    
    g_XorKey = AntiDetection::GenerateRandomKey();
    
    if (Hypervisor::IsVmxSupported()) {
        status = Hypervisor::InitializeHypervisor();
        if (NT_SUCCESS(status)) {
            Hypervisor::HideHypervisorPresence();
        }
    }
    
    PEHider::ErasePEHeaders(DriverObject);
    AntiDetection::SetDriverSectionInfo(DriverObject);
    
    g_DriverLoaded = TRUE;
    
    return STATUS_SUCCESS;
}

extern "C" VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    
    Hypervisor::CleanupHypervisor();
    
    if (g_SharedMemory) {
        Communication::CleanupSharedMemory(g_SharedMemory, g_SharedMemorySize);
        g_SharedMemory = nullptr;
    }
    
    AntiDetection::UnregisterHandleProtection();
    IoDeleteSymbolicLink(&g_SymbolicLink);
    
    if (g_DeviceObject) {
        IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = nullptr;
    }
    
    g_DriverLoaded = FALSE;
}