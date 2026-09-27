// File: xaloAC/Shared/DriverLoader.h
#pragma once
// xaloAC - Sürücü Yükleyici Başlık Dosyası (Güncellenmiş)

#include <windows.h>
#include <string>

namespace XaloDriverLoader {
    BOOL LoadDriverManually(const std::wstring& driverPath);
    BOOL UnloadDriver();
    BOOL IsDriverLoaded();
    PVOID MapSharedMemory();
    VOID UnmapSharedMemory(PVOID sharedMemory);
    BOOL SendCommand(UINT64 command, UINT64 processId, UINT64 sourceAddress, UINT64 targetAddress, UINT64 size, UINT64* status);
    std::string GetLastErrorMessage();
    UINT64 GetXorKey();
    HANDLE GetDriverHandle();
}