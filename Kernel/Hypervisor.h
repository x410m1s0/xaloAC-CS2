// File: xaloAC/Kernel/Hypervisor.h
#pragma once
// xaloAC - Hypervisor Tabanlı Koruma Başlık Dosyası (Düzeltilmiş)

#include <ntddk.h>

namespace Hypervisor {
    BOOLEAN IsVmxSupported();
    BOOLEAN IsSvmSupported();
    NTSTATUS InitializeHypervisor();
    void CleanupHypervisor();
    void HideHypervisorPresence();
    NTSTATUS HideMemoryWithEpt(PVOID Address, SIZE_T Size);
    NTSTATUS UnhideMemoryWithEpt(PVOID Address, SIZE_T Size);
    void MonitorVacAccess();
    BOOLEAN IsHypervisorActive();
}