// File: xaloAC/Shared/DriverLoader.cpp
// xaloAC - Sürücü Yükleyici Uygulaması (Düzeltilmiş ve Optimize Edilmiş)

#include "DriverLoader.h"
#include "XaloShared.h"
#include <winternl.h>
#include <sstream>
#include <vector>

namespace XaloDriverLoader {
    static std::string g_LastError = "";
    static PVOID g_MappedSharedMemory = nullptr;
    static HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;
    static UINT64 g_XorKey = 0;
    
    std::string GetLastErrorMessage() {
        return g_LastError;
    }
    
    BOOL IsDriverLoaded() {
        return g_DriverHandle != INVALID_HANDLE_VALUE;
    }
    
    PVOID MapSharedMemory() {
        g_DriverHandle = CreateFileW(
            L"\\\\.\\XaloACDriver",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        
        if (g_DriverHandle == INVALID_HANDLE_VALUE) {
            g_LastError = "Sürücü cihazı açılamadı. Sürücü yüklü mü?";
            return nullptr;
        }
        
        ULONG_PTR sharedMemoryAddress = 0;
        DWORD bytesReturned = 0;
        
        BOOL result = DeviceIoControl(
            g_DriverHandle,
            XALO_IOCTL_GET_SHARED_MEMORY,
            nullptr, 0,
            &sharedMemoryAddress, sizeof(sharedMemoryAddress),
            &bytesReturned, nullptr
        );
        
        if (!result || bytesReturned != sizeof(sharedMemoryAddress)) {
            g_LastError = "Shared memory adresi alınamadı.";
            CloseHandle(g_DriverHandle);
            g_DriverHandle = INVALID_HANDLE_VALUE;
            return nullptr;
        }
        
        // XOR anahtarını al
        UINT64 xorKey = 0;
        bytesReturned = 0;
        
        result = DeviceIoControl(
            g_DriverHandle,
            XALO_IOCTL_GET_XOR_KEY,
            nullptr, 0,
            &xorKey, sizeof(xorKey),
            &bytesReturned, nullptr
        );
        
        if (result && bytesReturned == sizeof(xorKey)) {
            g_XorKey = xorKey;
        }
        
        g_MappedSharedMemory = reinterpret_cast<PVOID>(sharedMemoryAddress);
        return g_MappedSharedMemory;
    }
    
    VOID UnmapSharedMemory(PVOID sharedMemory) {
        UNREFERENCED_PARAMETER(sharedMemory);
        
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(g_DriverHandle);
            g_DriverHandle = INVALID_HANDLE_VALUE;
        }
        
        g_MappedSharedMemory = nullptr;
    }
    
    BOOL SendCommand(UINT64 command, UINT64 processId, UINT64 sourceAddress, UINT64 targetAddress, UINT64 size, UINT64* status) {
        if (!g_MappedSharedMemory) {
            g_LastError = "Shared memory haritalanmamış.";
            return FALSE;
        }
        
        PXALO_RING_BUFFER ringBuffer = reinterpret_cast<PXALO_RING_BUFFER>(g_MappedSharedMemory);
        
        UINT64 writeIndex = ringBuffer->WriteIndex;
        UINT64 entryCount = sizeof(ringBuffer->Entries) / sizeof(ringBuffer->Entries[0]);
        
        PXALO_RING_BUFFER_ENTRY entry = &ringBuffer->Entries[writeIndex % entryCount];
        entry->Packet.Command = command;
        entry->Packet.ProcessId = processId;
        entry->Packet.SourceAddress = sourceAddress;
        entry->Packet.TargetAddress = targetAddress;
        entry->Packet.Size = size;
        entry->Packet.XorChecksum = command ^ processId ^ sourceAddress ^ targetAddress ^ size;
        entry->SequenceNumber = ringBuffer->SequenceNumber++;
        entry->Timestamp = GetTickCount64();
        
        _InterlockedIncrement64(reinterpret_cast<volatile LONG64*>(&ringBuffer->WriteIndex));
        _InterlockedIncrement64(reinterpret_cast<volatile LONG64*>(&ringBuffer->Count));
        
        UINT64 timeout = GetTickCount64() + 1000;
        
        while (GetTickCount64() < timeout) {
            if (entry->Packet.Status != 0) {
                if (status) {
                    *status = entry->Packet.Status;
                }
                return TRUE;
            }
            Sleep(0);
        }
        
        g_LastError = "Komut zaman aşımına uğradı.";
        return FALSE;
    }
    
    BOOL LoadDriverManually(const std::wstring& driverPath) {
        HANDLE fileHandle = CreateFileW(
            driverPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        
        if (fileHandle == INVALID_HANDLE_VALUE) {
            g_LastError = "Sürücü dosyası bulunamadı.";
            return FALSE;
        }
        
        DWORD fileSize = GetFileSize(fileHandle, nullptr);
        if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
            CloseHandle(fileHandle);
            g_LastError = "Sürücü dosyası boş.";
            return FALSE;
        }
        
        std::vector<BYTE> driverBuffer(fileSize);
        DWORD bytesRead = 0;
        
        if (!ReadFile(fileHandle, driverBuffer.data(), fileSize, &bytesRead, nullptr) || bytesRead != fileSize) {
            CloseHandle(fileHandle);
            g_LastError = "Sürücü dosyası okunamadı.";
            return FALSE;
        }
        CloseHandle(fileHandle);
        
        std::wstring serviceName = L"XaloACService";
        std::wstring keyPath = L"SYSTEM\\CurrentControlSet\\Services\\" + serviceName;
        
        HKEY hKey;
        if (RegCreateKeyW(HKEY_LOCAL_MACHINE, keyPath.c_str(), &hKey) != ERROR_SUCCESS) {
            g_LastError = "Registry anahtarı oluşturulamadı.";
            return FALSE;
        }
        
        DWORD type = 1;
        RegSetValueExW(hKey, L"Type", 0, REG_DWORD, reinterpret_cast<BYTE*>(&type), sizeof(type));
        
        DWORD start = 3;
        RegSetValueExW(hKey, L"Start", 0, REG_DWORD, reinterpret_cast<BYTE*>(&start), sizeof(start));
        
        std::wstring fullPath = L"\\??\\" + driverPath;
        RegSetValueExW(hKey, L"ImagePath", 0, REG_SZ, reinterpret_cast<const BYTE*>(fullPath.c_str()), static_cast<DWORD>((fullPath.size() + 1) * sizeof(wchar_t)));
        
        RegCloseKey(hKey);
        
        UNICODE_STRING servicePath;
        std::wstring ntPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\" + serviceName;
        
        servicePath.Buffer = const_cast<PWSTR>(ntPath.c_str());
        servicePath.Length = static_cast<USHORT>(ntPath.size() * sizeof(wchar_t));
        servicePath.MaximumLength = servicePath.Length + sizeof(wchar_t);
        
        typedef NTSTATUS(NTAPI* NtLoadDriver_t)(PUNICODE_STRING);
        static NtLoadDriver_t NtLoadDriver = reinterpret_cast<NtLoadDriver_t>(
            GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtLoadDriver")
        );
        
        if (!NtLoadDriver) {
            g_LastError = "NtLoadDriver bulunamadı.";
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, keyPath.c_str());
            return FALSE;
        }
        
        NTSTATUS status = NtLoadDriver(&servicePath);
        
        if (status != 0) {
            std::stringstream ss;
            ss << "Sürücü yüklenemedi. NTSTATUS: 0x" << std::hex << status;
            g_LastError = ss.str();
            RegDeleteKeyW(HKEY_LOCAL_MACHINE, keyPath.c_str());
            return FALSE;
        }
        
        RegDeleteKeyW(HKEY_LOCAL_MACHINE, keyPath.c_str());
        
        return TRUE;
    }
    
    BOOL UnloadDriver() {
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(g_DriverHandle);
            g_DriverHandle = INVALID_HANDLE_VALUE;
        }
        
        typedef NTSTATUS(NTAPI* NtUnloadDriver_t)(PUNICODE_STRING);
        static NtUnloadDriver_t NtUnloadDriver = reinterpret_cast<NtUnloadDriver_t>(
            GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtUnloadDriver")
        );
        
        if (NtUnloadDriver) {
            UNICODE_STRING servicePath;
            std::wstring ntPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\XaloACService";
            servicePath.Buffer = const_cast<PWSTR>(ntPath.c_str());
            servicePath.Length = static_cast<USHORT>(ntPath.size() * sizeof(wchar_t));
            servicePath.MaximumLength = servicePath.Length + sizeof(wchar_t);
            
            NtUnloadDriver(&servicePath);
        }
        
        return TRUE;
    }
    
    UINT64 GetXorKey() {
        return g_XorKey;
    }
    
    HANDLE GetDriverHandle() {
        return g_DriverHandle;
    }
}