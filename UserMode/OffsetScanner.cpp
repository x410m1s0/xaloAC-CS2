// File: xaloAC/UserMode/OffsetScanner.cpp
// xaloAC - Offset Tarayıcı Uygulaması

#include "OffsetScanner.h"
#include "XorStr.h"
#include <algorithm>

namespace XaloOffset {
    
    // Pattern tarama hız sınırı
    static DWORD g_MaxBytesPerSecond = 0x100000; // 1MB/sn
    
    ULONG_PTR FindPattern(HANDLE processHandle, ULONG_PTR moduleBase, SIZE_T moduleSize, const char* pattern, const char* mask) {
        if (!processHandle || processHandle == INVALID_HANDLE_VALUE || !pattern || !mask) {
            return 0;
        }
        
        SIZE_T patternLength = strlen(mask);
        
        if (patternLength == 0 || moduleSize < patternLength) {
            return 0;
        }
        
        // Modül belleğini oku
        std::vector<BYTE> moduleData(moduleSize);
        SIZE_T bytesRead = 0;
        
        // Hız sınırı uygula
        DWORD startTime = GetTickCount();
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(moduleBase), moduleData.data(), moduleSize, &bytesRead)) {
            return 0;
        }
        
        // Hız sınırı kontrolü
        DWORD elapsed = GetTickCount() - startTime;
        DWORD expectedTime = static_cast<DWORD>((moduleSize * 1000) / g_MaxBytesPerSecond);
        
        if (elapsed < expectedTime) {
            Sleep(expectedTime - elapsed);
        }
        
        if (bytesRead != moduleSize) {
            return 0;
        }
        
        // Pattern ara
        for (SIZE_T i = 0; i < moduleSize - patternLength; i++) {
            BOOL found = TRUE;
            
            for (SIZE_T j = 0; j < patternLength; j++) {
                if (mask[j] == 'x') {
                    if (moduleData[i + j] != static_cast<BYTE>(pattern[j])) {
                        found = FALSE;
                        break;
                    }
                }
            }
            
            if (found) {
                return moduleBase + i;
            }
        }
        
        return 0;
    }
    
    BOOL ValidateOffset(ULONG_PTR offset, ULONG_PTR moduleBase, SIZE_T moduleSize) {
        // Saçma değer kontrolü
        if (offset < moduleBase || offset >= moduleBase + moduleSize) {
            return FALSE;
        }
        
        // Offset 0 olmamalı
        if (offset == 0) {
            return FALSE;
        }
        
        // Offset page-aligned olmamalı (genellikle)
        // Bu kontrol bazı durumlarda yanlış pozitif verebilir
        
        return TRUE;
    }
    
    BOOL ScanSpecificOffset(HANDLE processHandle, ULONG_PTR moduleBase, SIZE_T moduleSize, const std::string& offsetName, const std::string& pattern, ULONG_PTR* OutOffset) {
        if (!processHandle || processHandle == INVALID_HANDLE_VALUE || !OutOffset) {
            return FALSE;
        }
        
        // Pattern ve mask'ı ayır
        std::string patternStr = pattern;
        std::string maskStr;
        
        // Pattern formatı: "48 8B 0D ? ? ? ? 48 85 C9 74 0A"
        // Mask formatı: "xxx????xxxx"
        
        std::vector<BYTE> patternBytes;
        std::vector<char> maskChars;
        
        size_t pos = 0;
        while (pos < patternStr.size()) {
            // Boşlukları atla
            while (pos < patternStr.size() && patternStr[pos] == ' ') pos++;
            
            if (pos >= patternStr.size()) break;
            
            // "?" kontrol et
            if (patternStr[pos] == '?') {
                patternBytes.push_back(0);
                maskChars.push_back('?');
                pos++;
                continue;
            }
            
            // Hex byte oku
            if (pos + 1 < patternStr.size()) {
                char hex[3] = {patternStr[pos], patternStr[pos + 1], 0};
                BYTE value = static_cast<BYTE>(strtol(hex, nullptr, 16));
                patternBytes.push_back(value);
                maskChars.push_back('x');
                pos += 2;
            } else {
                break;
            }
        }
        
        if (patternBytes.empty()) {
            return FALSE;
        }
        
        // Pattern'i string'e dönüştür
        std::string patternStr2(patternBytes.begin(), patternBytes.end());
        std::string maskStr2(maskChars.begin(), maskChars.end());
        
        // Pattern ara
        ULONG_PTR offset = FindPattern(processHandle, moduleBase, moduleSize, patternStr2.c_str(), maskStr2.c_str());
        
        if (!offset || !ValidateOffset(offset, moduleBase, moduleSize)) {
            *OutOffset = 0;
            return FALSE;
        }
        
        // Offset'i çözümle (relative offset'ler için)
        // Pattern genellikle "48 8B 0D ? ? ? ?" şeklindedir
        // Burada ? ? ? ? kısmı relative offset'tir
        
        // Basitleştirilmiş: Offset'i doğrudan döndür
        // Gerçek uygulamada: LEA/RIP-relative çözümleme yapılır
        
        *OutOffset = offset;
        return TRUE;
    }
    
    BOOL ScanAllOffsets(HANDLE processHandle, ULONG_PTR clientBase, SIZE_T clientSize, XALO_GAME_STATE* OutGameState) {
        if (!processHandle || processHandle == INVALID_HANDLE_VALUE || !OutGameState) {
            return FALSE;
        }
        
        BOOL allFound = TRUE;
        
        // dwEntityList offset'i
        ULONG_PTR entityListOffset = 0;
        if (ScanSpecificOffset(processHandle, clientBase, clientSize, 
            "dwEntityList", 
            "48 8B 0D ? ? ? ? 48 85 C9 74 0A", 
            &entityListOffset)) {
            OutGameState->OffsetEntityList = entityListOffset - clientBase;
        } else {
            allFound = FALSE;
        }
        
        // dwLocalPlayerPawn offset'i
        ULONG_PTR localPlayerPawnOffset = 0;
        if (ScanSpecificOffset(processHandle, clientBase, clientSize,
            "dwLocalPlayerPawn",
            "48 8B 05 ? ? ? ? 48 85 C0 74 12",
            &localPlayerPawnOffset)) {
            OutGameState->OffsetLocalPlayerPawn = localPlayerPawnOffset - clientBase;
        } else {
            allFound = FALSE;
        }
        
        // dwViewMatrix offset'i
        ULONG_PTR viewMatrixOffset = 0;
        if (ScanSpecificOffset(processHandle, clientBase, clientSize,
            "dwViewMatrix",
            "48 8D 0D ? ? ? ? 48 8B 15 ? ? ? ? F3 0F 10 42",
            &viewMatrixOffset)) {
            OutGameState->OffsetViewMatrix = viewMatrixOffset - clientBase;
        } else {
            allFound = FALSE;
        }
        
        // Ek offset'ler
        // m_pGameSceneNode - genellikle 0x310 civarı
        OutGameState->OffsetGameSceneNode = 0x310;
        
        // m_modelState - genellikle 0x170 civarı
        OutGameState->OffsetModelState = 0x170;
        
        // Bone array - genellikle 0x1E0 civarı
        OutGameState->OffsetBoneArray = 0x1E0;
        
        // Eğer bazı offset'ler bulunamadıysa yedekleri yükle
        if (!allFound) {
            LoadBackupOffsets(OutGameState);
        }
        
        return allFound;
    }
    
    void LoadBackupOffsets(XALO_GAME_STATE* OutGameState) {
        if (!OutGameState) {
            return;
        }
        
        // Yedek offset'ler (CS2'nin son güncellemeleri için)
        // Bu değerler manuel olarak güncellenir
        
        if (OutGameState->OffsetEntityList == 0) {
            OutGameState->OffsetEntityList = 0x1A0B5A0; // Örnek değer
        }
        
        if (OutGameState->OffsetLocalPlayerPawn == 0) {
            OutGameState->OffsetLocalPlayerPawn = 0x187AB48; // Örnek değer
        }
        
        if (OutGameState->OffsetViewMatrix == 0) {
            OutGameState->OffsetViewMatrix = 0x1A1D2C0; // Örnek değer
        }
    }
    
    BOOL AreOffsetsValid(HANDLE processHandle, const XALO_GAME_STATE& gameState) {
        if (!processHandle || processHandle == INVALID_HANDLE_VALUE) {
            return FALSE;
        }
        
        // Offset'lerin geçerli olup olmadığını kontrol et
        if (gameState.OffsetEntityList == 0 ||
            gameState.OffsetLocalPlayerPawn == 0 ||
            gameState.OffsetViewMatrix == 0) {
            return FALSE;
        }
        
        // Entity listesini okumayı dene
        ULONG_PTR entityListAddress = gameState.ClientBaseAddress + gameState.OffsetEntityList;
        ULONG_PTR entityList = 0;
        
        if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(entityListAddress), &entityList, sizeof(entityList), nullptr)) {
            return FALSE;
        }
        
        // Entity listesi 0 olmamalı
        if (entityList == 0) {
            return FALSE;
        }
        
        return TRUE;
    }
    
    BOOL RescanOffsets(HANDLE processHandle, ULONG_PTR clientBase, SIZE_T clientSize, XALO_GAME_STATE* OutGameState) {
        // Offset'leri yeniden tara
        return ScanAllOffsets(processHandle, clientBase, clientSize, OutGameState);
    }
    
    void SetScanSpeedLimit(DWORD maxBytesPerSecond) {
        g_MaxBytesPerSecond = maxBytesPerSecond;
    }
}