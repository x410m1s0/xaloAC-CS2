// File: xaloAC/Kernel/AntiDetection.h
#pragma once
// xaloAC - Anti-Tespit Katmanı Başlık Dosyası

#include <ntddk.h>
#include <ntstrsafe.h>

namespace AntiDetection {
    // ============ DEBUGGER TESPİTİ ============
    BOOLEAN IsKernelDebuggerPresent();
    
    // ============ SÜRÜCÜ GİZLEME ============
    void HideDriverFromPsLoadedModuleList(PDRIVER_OBJECT DriverObject);
    void CleanPiDDBCacheTable(PDRIVER_OBJECT DriverObject);
    void CleanMmUnloadedDrivers(PDRIVER_OBJECT DriverObject);
    void UnlinkDriverSection(PDRIVER_OBJECT DriverObject);
    void HideDriverObject(PDRIVER_OBJECT DriverObject);
    void HideDeviceObject(PDEVICE_OBJECT DeviceObject);
    void SetDriverSectionInfo(PDRIVER_OBJECT DriverObject);
    
    // ============ HANDLE KORUMASI ============
    NTSTATUS RegisterHandleProtection();
    void UnregisterHandleProtection();
    
    // ============ TRACING/ETW GİZLEME ============
    void DisableWppTracing();
    void HideEtwEvents();
    
    // ============ NtQueryVirtualMemory HOOK ============
    NTSTATUS HookNtQueryVirtualMemory();
    void UnhookNtQueryVirtualMemory();
    
    // ============ RASTGELE ANAHTAR ============
    UINT64 GenerateRandomKey();
    
    // ============ VAC TARAMA TESPİTİ ============
    BOOLEAN IsVacScanning();
    void SetVacScanningState(BOOLEAN scanning);
    
    // ============ SÜRÜCÜ ŞİFRELEME ============
    void EncryptDriverSection();
    void DecryptDriverSection();
    
    // ============ STACK OBFUSKASYON ============
    void ObfuscateStack();
    
    // ============ SAYFA TABLOSU GİZLEME ============
    void HideDriverFromPageTables();
    void ShowDriverFromPageTables();
}