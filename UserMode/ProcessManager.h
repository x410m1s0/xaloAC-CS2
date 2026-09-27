// File: xaloAC/UserMode/ProcessManager.h
#pragma once
// xaloAC - Process Yöneticisi Başlık Dosyası

#include <windows.h>
#include <string>
#include <vector>
#include "../Shared/XaloShared.h"

namespace XaloProcess {
    // CS2 process'ini bul
    BOOL FindCS2Process(DWORD* OutProcessId);
    
    // CS2 process'ini bekle (başlatılana kadar)
    BOOL WaitForCS2Process(DWORD* OutProcessId, DWORD timeoutMs = 60000);
    
    // Process handle'ı aç
    HANDLE OpenCS2Process(DWORD processId);
    
    // Process handle'ı kapat
    void CloseCS2Process(HANDLE processHandle);
    
    // Modül bilgisini al (kernel sürücüsü üzerinden)
    BOOL GetModuleInfoFromKernel(DWORD processId, const std::wstring& moduleName, ULONG_PTR* OutBaseAddress, SIZE_T* OutSize);
    
    // Process modüllerini listele
    BOOL ListProcessModules(DWORD processId, std::vector<MODULEENTRY32W>& OutModules);
    
    // CS2 güncellemelerini kontrol et
    BOOL CheckForGameUpdates(DWORD processId);
    
    // Process adını gizle
    void HideProcessName();
    
    // Process bilgisini al
    BOOL GetProcessInfo(DWORD processId, XALO_GAME_STATE* OutGameState);
    
    // Bellek okuma (kernel üzerinden)
    BOOL ReadMemoryKernel(HANDLE driverHandle, DWORD processId, ULONG_PTR address, PVOID buffer, SIZE_T size);
    
    // Bellek yazma (kernel üzerinden)
    BOOL WriteMemoryKernel(HANDLE driverHandle, DWORD processId, ULONG_PTR address, PVOID buffer, SIZE_T size);
}