// File: xaloAC/Kernel/MemoryOps.cpp
// xaloAC - Bellek Operasyonları Uygulaması (Düzeltilmiş ve Optimize Edilmiş)

#include "MemoryOps.h"
#include "AntiDetection.h"
#include "../Shared/XaloShared.h"

// Fiziksel bellek okuma/yazma yardımcı fonksiyonları
static void ReadPhysicalAddress(ULONG_PTR PhysicalAddress, PVOID Buffer, SIZE_T Size) {
    PHYSICAL_ADDRESS physAddr;
    physAddr.QuadPart = PhysicalAddress;
    
    PVOID mapped = MmMapIoSpace(physAddr, Size, MmNonCached);
    if (mapped) {
        RtlCopyMemory(Buffer, mapped, Size);
        MmUnmapIoSpace(mapped, Size);
    }
}

static void WritePhysicalAddress(ULONG_PTR PhysicalAddress, PVOID Buffer, SIZE_T Size) {
    PHYSICAL_ADDRESS physAddr;
    physAddr.QuadPart = PhysicalAddress;
    
    PVOID mapped = MmMapIoSpace(physAddr, Size, MmNonCached);
    if (mapped) {
        RtlCopyMemory(mapped, Buffer, Size);
        MmUnmapIoSpace(mapped, Size);
    }
}

namespace MemoryOps {
    
    NTSTATUS GetProcessById(HANDLE ProcessId, PEPROCESS* OutProcess) {
        if (!OutProcess) {
            return STATUS_INVALID_PARAMETER;
        }
        
        NTSTATUS status = PsLookupProcessByProcessId(ProcessId, OutProcess);
        if (NT_SUCCESS(status)) {
            return status;
        }
        
        // Alternatif: Sistem process listesinde ara
        ULONG bufferSize = 0;
        ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &bufferSize);
        
        if (bufferSize == 0) {
            return STATUS_NOT_FOUND;
        }
        
        PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, XALO_DRIVER_TAG);
        if (!buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
        status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, nullptr);
        if (NT_SUCCESS(status)) {
            PSYSTEM_PROCESS_INFORMATION processInfo = static_cast<PSYSTEM_PROCESS_INFORMATION>(buffer);
            
            while (true) {
                if (reinterpret_cast<HANDLE>(processInfo->UniqueProcessId) == ProcessId) {
                    status = PsLookupProcessByProcessId(ProcessId, OutProcess);
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
        return status;
    }
    
    NTSTATUS ReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetBuffer, SIZE_T Size) {
        if (!Process || !SourceAddress || !TargetBuffer || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        SIZE_T bytesRead = 0;
        NTSTATUS status = MmCopyVirtualMemory(
            Process,
            SourceAddress,
            PsGetCurrentProcess(),
            TargetBuffer,
            Size,
            KernelMode,
            &bytesRead
        );
        
        if (NT_SUCCESS(status) && bytesRead == Size) {
            return STATUS_SUCCESS;
        }
        
        KAPC_STATE apcState;
        KeStackAttachProcess(Process, &apcState);
        
        __try {
            ProbeForRead(SourceAddress, Size, 1);
            RtlCopyMemory(TargetBuffer, SourceAddress, Size);
            status = STATUS_SUCCESS;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        
        KeUnstackDetachProcess(&apcState);
        return status;
    }
    
    NTSTATUS WriteVirtualMemory(PEPROCESS Process, PVOID TargetAddress, PVOID SourceBuffer, SIZE_T Size) {
        if (!Process || !TargetAddress || !SourceBuffer || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        SIZE_T bytesWritten = 0;
        NTSTATUS status = MmCopyVirtualMemory(
            PsGetCurrentProcess(),
            SourceBuffer,
            Process,
            TargetAddress,
            Size,
            KernelMode,
            &bytesWritten
        );
        
        if (NT_SUCCESS(status) && bytesWritten == Size) {
            return STATUS_SUCCESS;
        }
        
        KAPC_STATE apcState;
        KeStackAttachProcess(Process, &apcState);
        
        __try {
            ProbeForWrite(TargetAddress, Size, 1);
            RtlCopyMemory(TargetAddress, SourceBuffer, Size);
            status = STATUS_SUCCESS;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        
        KeUnstackDetachProcess(&apcState);
        return status;
    }
    
    NTSTATUS GetModuleBaseAddress(PEPROCESS Process, const wchar_t* ModuleName, PVOID* OutBaseAddress, SIZE_T* OutSize) {
        if (!Process || !ModuleName || !OutBaseAddress) {
            return STATUS_INVALID_PARAMETER;
        }
        
        NTSTATUS status = STATUS_NOT_FOUND;
        KAPC_STATE apcState;
        KeStackAttachProcess(Process, &apcState);
        
        __try {
            PPEB peb = PsGetProcessPeb(Process);
            if (!peb) {
                status = STATUS_NOT_FOUND;
                goto cleanup;
            }
            
            PPEB_LDR_DATA ldr = peb->Ldr;
            if (!ldr) {
                status = STATUS_NOT_FOUND;
                goto cleanup;
            }
            
            PLIST_ENTRY listHead = &ldr->InLoadOrderModuleList;
            PLIST_ENTRY listEntry = listHead->Flink;
            
            UNICODE_STRING targetName;
            RtlInitUnicodeString(&targetName, ModuleName);
            
            while (listEntry != listHead) {
                PLDR_DATA_TABLE_ENTRY moduleEntry = CONTAINING_RECORD(
                    listEntry,
                    LDR_DATA_TABLE_ENTRY,
                    InLoadOrderModuleList
                );
                
                if (moduleEntry->BaseDllName.Buffer) {
                    if (RtlEqualUnicodeString(&moduleEntry->BaseDllName, &targetName, TRUE)) {
                        *OutBaseAddress = moduleEntry->DllBase;
                        if (OutSize) {
                            *OutSize = moduleEntry->SizeOfImage;
                        }
                        status = STATUS_SUCCESS;
                        break;
                    }
                }
                
                listEntry = listEntry->Flink;
                if (listEntry == listHead) break;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        
cleanup:
        KeUnstackDetachProcess(&apcState);
        return status;
    }
    
    NTSTATUS ChangeMemoryProtection(PEPROCESS Process, PVOID Address, SIZE_T Size, ULONG NewProtection, ULONG* OldProtection) {
        if (!Process || !Address || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        KAPC_STATE apcState;
        KeStackAttachProcess(Process, &apcState);
        
        NTSTATUS status = STATUS_SUCCESS;
        
        __try {
            PMDL mdl = IoAllocateMdl(Address, static_cast<ULONG>(Size), FALSE, FALSE, nullptr);
            if (!mdl) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
            
            MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
            
            if (OldProtection) {
                *OldProtection = mdl->MdlFlags & 0x1F;
            }
            
            MmProtectMdlPages(mdl, NewProtection);
            MmUnlockPages(mdl);
            IoFreeMdl(mdl);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        
cleanup:
        KeUnstackDetachProcess(&apcState);
        return status;
    }
    
    NTSTATUS ReadLargeDataBlock(PEPROCESS Process, PVOID Address, PVOID Buffer, SIZE_T Size) {
        if (!Process || !Address || !Buffer || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        const SIZE_T MAX_CHUNK_SIZE = PAGE_SIZE * 64;
        
        if (Size <= MAX_CHUNK_SIZE) {
            return ReadVirtualMemory(Process, Address, Buffer, Size);
        }
        
        SIZE_T totalRead = 0;
        ULONG_PTR currentAddress = reinterpret_cast<ULONG_PTR>(Address);
        PUCHAR currentBuffer = static_cast<PUCHAR>(Buffer);
        
        while (totalRead < Size) {
            SIZE_T chunkSize = min(MAX_CHUNK_SIZE, Size - totalRead);
            
            NTSTATUS status = ReadVirtualMemory(
                Process,
                reinterpret_cast<PVOID>(currentAddress),
                currentBuffer,
                chunkSize
            );
            
            if (!NT_SUCCESS(status)) {
                return status;
            }
            
            currentAddress += chunkSize;
            currentBuffer += chunkSize;
            totalRead += chunkSize;
        }
        
        return STATUS_SUCCESS;
    }
    
    NTSTATUS HideMemoryRegion(PEPROCESS Process, PVOID Address, SIZE_T Size) {
        if (!Process || !Address || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        ULONG_PTR cr3 = reinterpret_cast<ULONG_PTR>(PsGetProcessPageDirectory(Process));
        if (!cr3) {
            return STATUS_NOT_FOUND;
        }
        
        ULONG_PTR virtualAddress = reinterpret_cast<ULONG_PTR>(Address);
        SIZE_T pagesToHide = (Size + PAGE_SIZE - 1) / PAGE_SIZE;
        
        for (SIZE_T i = 0; i < pagesToHide; i++) {
            ULONG_PTR currentAddress = virtualAddress + (i * PAGE_SIZE);
            
            ULONG_PTR pml4Index = (currentAddress >> 39) & 0x1FF;
            ULONG_PTR pdptIndex = (currentAddress >> 30) & 0x1FF;
            ULONG_PTR pdIndex = (currentAddress >> 21) & 0x1FF;
            ULONG_PTR ptIndex = (currentAddress >> 12) & 0x1FF;
            
            ULONG_PTR pml4e = 0;
            ReadPhysicalAddress(cr3 + pml4Index * 8, &pml4e, 8);
            if (!(pml4e & 1)) continue;
            
            ULONG_PTR pdpte = 0;
            ReadPhysicalAddress((pml4e & 0xFFFFFFFFFF000ULL) + pdptIndex * 8, &pdpte, 8);
            if (!(pdpte & 1)) continue;
            
            ULONG_PTR pde = 0;
            ReadPhysicalAddress((pdpte & 0xFFFFFFFFFF000ULL) + pdIndex * 8, &pde, 8);
            if (!(pde & 1)) continue;
            
            ULONG_PTR pteAddress = (pde & 0xFFFFFFFFFF000ULL) + ptIndex * 8;
            ULONG_PTR pte = 0;
            ReadPhysicalAddress(pteAddress, &pte, 8);
            
            if (pte & 1) {
                pte &= ~1ULL;
                WritePhysicalAddress(pteAddress, &pte, 8);
                __invlpg(reinterpret_cast<PVOID>(currentAddress));
            }
        }
        
        return STATUS_SUCCESS;
    }
    
    NTSTATUS UnhideMemoryRegion(PEPROCESS Process, PVOID Address, SIZE_T Size) {
        if (!Process || !Address || Size == 0) {
            return STATUS_INVALID_PARAMETER;
        }
        
        ULONG_PTR cr3 = reinterpret_cast<ULONG_PTR>(PsGetProcessPageDirectory(Process));
        if (!cr3) {
            return STATUS_NOT_FOUND;
        }
        
        ULONG_PTR virtualAddress = reinterpret_cast<ULONG_PTR>(Address);
        SIZE_T pagesToRestore = (Size + PAGE_SIZE - 1) / PAGE_SIZE;
        
        for (SIZE_T i = 0; i < pagesToRestore; i++) {
            ULONG_PTR currentAddress = virtualAddress + (i * PAGE_SIZE);
            
            ULONG_PTR pml4Index = (currentAddress >> 39) & 0x1FF;
            ULONG_PTR pdptIndex = (currentAddress >> 30) & 0x1FF;
            ULONG_PTR pdIndex = (currentAddress >> 21) & 0x1FF;
            ULONG_PTR ptIndex = (currentAddress >> 12) & 0x1FF;
            
            ULONG_PTR pml4e = 0;
            ReadPhysicalAddress(cr3 + pml4Index * 8, &pml4e, 8);
            if (!(pml4e & 1)) continue;
            
            ULONG_PTR pdpte = 0;
            ReadPhysicalAddress((pml4e & 0xFFFFFFFFFF000ULL) + pdptIndex * 8, &pdpte, 8);
            if (!(pdpte & 1)) continue;
            
            ULONG_PTR pde = 0;
            ReadPhysicalAddress((pdpte & 0xFFFFFFFFFF000ULL) + pdIndex * 8, &pde, 8);
            if (!(pde & 1)) continue;
            
            ULONG_PTR pteAddress = (pde & 0xFFFFFFFFFF000ULL) + ptIndex * 8;
            ULONG_PTR pte = 0;
            ReadPhysicalAddress(pteAddress, &pte, 8);
            
            pte |= 1ULL;
            WritePhysicalAddress(pteAddress, &pte, 8);
            __invlpg(reinterpret_cast<PVOID>(currentAddress));
        }
        
        return STATUS_SUCCESS;
    }
    
    NTSTATUS ProtectPagesWithMdl(PVOID Address, SIZE_T Size, ULONG NewProtection) {
        PMDL mdl = IoAllocateMdl(Address, static_cast<ULONG>(Size), FALSE, FALSE, nullptr);
        if (!mdl) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
        MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
        MmProtectMdlPages(mdl, NewProtection);
        MmUnlockPages(mdl);
        IoFreeMdl(mdl);
        
        return STATUS_SUCCESS;
    }
}