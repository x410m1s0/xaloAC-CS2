// File: xaloAC/Kernel/MemoryOps.h
#pragma once
// xaloAC - Bellek Operasyonları Başlık Dosyası

#include <ntddk.h>
#include <ntifs.h>

namespace MemoryOps {
    // Process ID'den PEPROCESS çözümle
    NTSTATUS GetProcessById(HANDLE ProcessId, PEPROCESS* OutProcess);
    
    // User-mode adres alanından okuma
    NTSTATUS ReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetBuffer, SIZE_T Size);
    
    // User-mode adres alanına yazma
    NTSTATUS WriteVirtualMemory(PEPROCESS Process, PVOID TargetAddress, PVOID SourceBuffer, SIZE_T Size);
    
    // Modül base adresini bul
    NTSTATUS GetModuleBaseAddress(PEPROCESS Process, const wchar_t* ModuleName, PVOID* OutBaseAddress, SIZE_T* OutSize);
    
    // Sanal bellek koruması değiştir
    NTSTATUS ChangeMemoryProtection(PEPROCESS Process, PVOID Address, SIZE_T Size, ULONG NewProtection, ULONG* OldProtection);
    
    // Büyük veri bloğu okuma
    NTSTATUS ReadLargeDataBlock(PEPROCESS Process, PVOID Address, PVOID Buffer, SIZE_T Size);
    
    // Sayfa tablosu ile bellek gizle
    NTSTATUS HideMemoryRegion(PEPROCESS Process, PVOID Address, SIZE_T Size);
    NTSTATUS UnhideMemoryRegion(PEPROCESS Process, PVOID Address, SIZE_T Size);
    
    // MDL ile sayfa koruması
    NTSTATUS ProtectPagesWithMdl(PVOID Address, SIZE_T Size, ULONG NewProtection);
    
    // Fiziksel bellek yardımcıları
    void ReadPhysicalAddress(ULONG_PTR PhysicalAddress, PVOID Buffer, SIZE_T Size);
    void WritePhysicalAddress(ULONG_PTR PhysicalAddress, PVOID Buffer, SIZE_T Size);
    
    // Process modül listesini al
    NTSTATUS GetProcessModules(PEPROCESS Process, PVOID* ModuleList, ULONG* ModuleCount);
}