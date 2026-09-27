// File: xaloAC/UserMode/ProcessManager.cpp
// xaloAC - Process Yöneticisi Uygulaması (Düzeltilmiş - IOCTL Kodları Güncellendi)

#include "ProcessManager.h"
#include "XorStr.h"
#include "../Shared/XaloShared.h"
#include <TlHelp32.h>
#include <winternl.h>

namespace XaloProcess {
    
    BOOL FindCS2Process(DWORD* OutProcessId) {
        if (!OutProcessId) return FALSE;
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return FALSE;
        
        PROCESSENTRY32W processEntry = {0};
        processEntry.dwSize = sizeof(processEntry);
        
        BOOL found = FALSE;
        
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, L"cs2.exe") == 0) {
                    *OutProcessId = processEntry.th32ProcessID;
                    found = TRUE;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        
        CloseHandle(snapshot);
        return found;
    }
    
    BOOL WaitForCS2Process(DWORD* OutProcessId, DWORD timeoutMs) {
        if (!OutProcessId) return FALSE;
        
        DWORD startTime = GetTickCount();
        
        while (GetTickCount() - startTime < timeoutMs) {
            if (FindCS2Process(OutProcessId)) {
                return TRUE;
            }
            Sleep(1000);
        }
        
        return FALSE;
    }
    
    HANDLE OpenCS2Process(DWORD processId) {
        return OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
            FALSE,
            processId
        );
    }
    
    void CloseCS2Process(HANDLE processHandle) {
        if (processHandle && processHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(processHandle);
        }
    }
    
    BOOL GetModuleInfoFromKernel(DWORD processId, const std::wstring& moduleName, ULONG_PTR* OutBaseAddress, SIZE_T* OutSize) {
        HANDLE driverHandle = CreateFileW(
            L"\\\\.\\XaloACDriver",
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        
        if (driverHandle == INVALID_HANDLE_VALUE) {
            return FALSE;
        }
        
        // Modül adı + ProcessId içeren buffer
        SIZE_T moduleNameSize = (moduleName.size() + 1) * sizeof(wchar_t);
        SIZE_T bufferSize = moduleNameSize + sizeof(HANDLE);
        
        std::vector<BYTE> inputBuffer(bufferSize);
        memcpy(inputBuffer.data(), moduleName.c_str(), moduleNameSize);
        *reinterpret_cast<HANDLE*>(inputBuffer.data() + moduleNameSize) = reinterpret_cast<HANDLE>(processId);
        
        struct ModuleResult {
            ULONG_PTR BaseAddress;
            SIZE_T Size;
        } result;
        
        DWORD bytesReturned = 0;
        
        BOOL success = DeviceIoControl(
            driverHandle,
            XALO_IOCTL_GET_MODULE_BASE,
            inputBuffer.data(),
            static_cast<DWORD>(bufferSize),
            &result,
            sizeof(result),
            &bytesReturned,
            nullptr
        );
        
        CloseHandle(driverHandle);
        
        if (!success || bytesReturned != sizeof(result)) {
            return FALSE;
        }
        
        if (OutBaseAddress) *OutBaseAddress = result.BaseAddress;
        if (OutSize) *OutSize = result.Size;
        
        return TRUE;
    }
    
    BOOL ListProcessModules(DWORD processId, std::vector<MODULEENTRY32W>& OutModules) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) return FALSE;
        
        MODULEENTRY32W moduleEntry = {0};
        moduleEntry.dwSize = sizeof(moduleEntry);
        
        OutModules.clear();
        
        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                OutModules.push_back(moduleEntry);
            } while (Module32NextW(snapshot, &moduleEntry));
        }
        
        CloseHandle(snapshot);
        return !OutModules.empty();
    }
    
    BOOL CheckForGameUpdates(DWORD processId) {
        std::vector<MODULEENTRY32W> modules;
        if (!ListProcessModules(processId, modules)) return FALSE;
        
        for (const auto& module : modules) {
            if (_wcsicmp(module.szModule, L"client.dll") == 0) {
                return TRUE;
            }
        }
        
        return FALSE;
    }
    
    void HideProcessName() {
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        if (!peb || !peb->ProcessParameters) return;
    }
    
    BOOL GetProcessInfo(DWORD processId, XALO_GAME_STATE* OutGameState) {
        if (!OutGameState) return FALSE;
        
        HANDLE processHandle = OpenCS2Process(processId);
        if (!processHandle || processHandle == INVALID_HANDLE_VALUE) return FALSE;
        
        std::vector<MODULEENTRY32W> modules;
        if (!ListProcessModules(processId, modules)) {
            CloseCS2Process(processHandle);
            return FALSE;
        }
        
        for (const auto& module : modules) {
            if (_wcsicmp(module.szModule, L"client.dll") == 0) {
                OutGameState->ClientBaseAddress = reinterpret_cast<UINT64>(module.modBaseAddr);
                OutGameState->ClientSize = module.modBaseSize;
            } else if (_wcsicmp(module.szModule, L"engine2.dll") == 0) {
                OutGameState->EngineBaseAddress = reinterpret_cast<UINT64>(module.modBaseAddr);
                OutGameState->EngineSize = module.modBaseSize;
            } else if (_wcsicmp(module.szModule, L"matchmaking.dll") == 0) {
                OutGameState->MatchmakingBaseAddress = reinterpret_cast<UINT64>(module.modBaseAddr);
                OutGameState->MatchmakingSize = module.modBaseSize;
            }
        }
        
        OutGameState->Cs2ProcessId = processId;
        
        CloseCS2Process(processHandle);
        return TRUE;
    }
    
    BOOL ReadMemoryKernel(HANDLE driverHandle, DWORD processId, ULONG_PTR address, PVOID buffer, SIZE_T size) {
        if (!driverHandle || driverHandle == INVALID_HANDLE_VALUE || !buffer || size == 0) {
            return FALSE;
        }
        
        struct ReadRequest {
            HANDLE ProcessId;
            ULONG_PTR Address;
            SIZE_T Size;
        } request;
        
        request.ProcessId = reinterpret_cast<HANDLE>(processId);
        request.Address = address;
        request.Size = size;
        
        DWORD bytesReturned = 0;
        
        BOOL result = DeviceIoControl(
            driverHandle,
            XALO_IOCTL_READ_MEMORY,
            &request,
            sizeof(request),
            buffer,
            static_cast<DWORD>(size),
            &bytesReturned,
            nullptr
        );
        
        return result && bytesReturned == size;
    }
    
    BOOL WriteMemoryKernel(HANDLE driverHandle, DWORD processId, ULONG_PTR address, PVOID buffer, SIZE_T size) {
        if (!driverHandle || driverHandle == INVALID_HANDLE_VALUE || !buffer || size == 0) {
            return FALSE;
        }
        
        struct WriteRequest {
            HANDLE ProcessId;
            ULONG_PTR Address;
            SIZE_T Size;
            BYTE Data[512];
        } request;
        
        request.ProcessId = reinterpret_cast<HANDLE>(processId);
        request.Address = address;
        request.Size = size;
        
        SIZE_T copySize = min(size, sizeof(request.Data));
        memcpy(request.Data, buffer, copySize);
        
        DWORD bytesReturned = 0;
        
        BOOL result = DeviceIoControl(
            driverHandle,
            XALO_IOCTL_WRITE_MEMORY,
            &request,
            sizeof(request),
            nullptr,
            0,
            &bytesReturned,
            nullptr
        );
        
        return result;
    }
}