// File: xaloAC/UserMode/OffsetScanner.h
#pragma once
// xaloAC - Offset Tarayıcı Başlık Dosyası

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include "../Shared/XaloShared.h"

namespace XaloOffset {
    // Offset bilgisi
    struct OffsetInfo {
        std::string Name;
        ULONG_PTR Offset;
        BOOL IsFound;
        std::string Pattern;
    };
    
    // Pattern tarama ile offset bul
    ULONG_PTR FindPattern(HANDLE processHandle, ULONG_PTR moduleBase, SIZE_T moduleSize, const char* pattern, const char* mask);
    
    // Tüm offset'leri tara
    BOOL ScanAllOffsets(HANDLE processHandle, ULONG_PTR clientBase, SIZE_T clientSize, XALO_GAME_STATE* OutGameState);
    
    // Belirli bir offset'i tara
    BOOL ScanSpecificOffset(HANDLE processHandle, ULONG_PTR moduleBase, SIZE_T moduleSize, const std::string& offsetName, const std::string& pattern, ULONG_PTR* OutOffset);
    
    // Offset değerini doğrula (saçma değer kontrolü)
    BOOL ValidateOffset(ULONG_PTR offset, ULONG_PTR moduleBase, SIZE_T moduleSize);
    
    // Yedek offset'leri yükle
    void LoadBackupOffsets(XALO_GAME_STATE* OutGameState);
    
    // Offset'lerin güncel olup olmadığını kontrol et
    BOOL AreOffsetsValid(HANDLE processHandle, const XALO_GAME_STATE& gameState);
    
    // Offset'leri yeniden tara
    BOOL RescanOffsets(HANDLE processHandle, ULONG_PTR clientBase, SIZE_T clientSize, XALO_GAME_STATE* OutGameState);
    
    // Pattern tarama hız sınırı (VAC'in dikkatini çekmemek için)
    void SetScanSpeedLimit(DWORD maxBytesPerSecond);
}