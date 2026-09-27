// File: xaloAC/UserMode/ConfigManager.h
#pragma once
// xaloAC - Konfigürasyon Yöneticisi Başlık Dosyası

#include <string>
#include <atomic>
#include <mutex>
#include "../Shared/XaloShared.h"

namespace XaloConfig {
    // Konfigürasyonu yükle
    BOOL LoadConfig(const std::string& configPath);
    
    // Konfigürasyonu kaydet
    BOOL SaveConfig(const std::string& configPath);
    
    // Varsayılan konfigürasyonu oluştur
    void CreateDefaultConfig();
    
    // Konfigürasyonu al (thread-safe)
    XALO_CONFIG GetConfig();
    
    // Konfigürasyonu güncelle (thread-safe)
    void UpdateConfig(const XALO_CONFIG& newConfig);
    
    // Belirli bir ayarı güncelle
    void SetConfigValue(UINT32 offset, UINT32 value);
    
    // Belirli bir ayarı al
    UINT32 GetConfigValue(UINT32 offset);
    
    // Profil yükle
    void LoadProfile(UINT32 profileIndex);
    
    // Rage profili
    void LoadRageProfile();
    
    // Legit profili
    void LoadLegitProfile();
    
    // Custom profili
    void LoadCustomProfile();
    
    // Konfigürasyon değişti mi?
    BOOL IsConfigDirty();
    
    // Konfigürasyonu temizle
    void ClearConfig();
}