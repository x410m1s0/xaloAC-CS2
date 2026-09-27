// File: xaloAC/UserMode/ConfigManager.cpp
// xaloAC - Konfigürasyon Yöneticisi Uygulaması

#include "ConfigManager.h"
#include "XorStr.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace XaloConfig {
    static XALO_CONFIG g_CurrentConfig = {0};
    static std::mutex g_ConfigMutex;
    static std::atomic<BOOL> g_ConfigDirty = FALSE;
    static std::string g_ConfigPath = "";
    
    void CreateDefaultConfig() {
        XALO_CONFIG config = {0};
        
        // ESP varsayılanları
        config.ESPEnabled = 1;
        config.ESPBoxEnabled = 1;
        config.ESPHealthBarEnabled = 1;
        config.ESPNameEnabled = 1;
        config.ESPDistanceEnabled = 1;
        config.ESPHeadDotEnabled = 1;
        config.ESPLineEnabled = 0;
        config.ESPSnaplineEnabled = 0;
        config.ESPVisibleCheck = 1;
        config.ESPTeamCheck = 1;
        config.ESPEnemyOnly = 1;
        config.ESPGlowEnabled = 0;
        
        // ESP renkler - görünür oyuncular (yeşil)
        config.ESPVisibleColor = {0.0f, 1.0f, 0.0f, 1.0f};
        // ESP renkler - gizli oyuncular (kırmızı)
        config.ESPHiddenColor = {1.0f, 0.0f, 0.0f, 1.0f};
        config.ESPBoxColor = {0.0f, 1.0f, 0.0f, 1.0f};
        config.ESPHealthBarColor = {0.0f, 1.0f, 0.0f, 1.0f};
        config.ESPNameColor = {1.0f, 1.0f, 1.0f, 1.0f};
        config.ESPLineColor = {0.5f, 0.5f, 1.0f, 0.7f};
        config.ESPHeadDotColor = {1.0f, 1.0f, 0.0f, 1.0f};
        
        config.ESPBoxThickness = 1.5f;
        config.ESPLineThickness = 1.0f;
        config.ESPMaxDistance = 100.0f;
        
        // Aimbot varsayılanları
        config.AimbotEnabled = 1;
        config.AimbotTargetMode = 0; // Crosshair
        config.AimbotBoneTarget = 0; // Kafa
        config.AimbotVisibleCheck = 1;
        config.AimbotTeamCheck = 1;
        config.AimbotSmoothingEnabled = 1;
        config.AimbotSmoothingFactor = 5.0f;
        config.AimbotFOV = 90.0f;
        config.AimbotRecoilControl = 1;
        config.AimbotRecoilStrength = 0.5f;
        config.AimbotLockMode = 0; // Toggle
        config.AimbotKey = VK_LBUTTON; // Sol tık
        config.AimbotAutoTarget = 1;
        
        // Triggerbot varsayılanları
        config.TriggerbotEnabled = 0;
        config.TriggerbotKey = VK_MENU; // ALT
        config.TriggerbotMinDelay = 50.0f;
        config.TriggerbotMaxDelay = 200.0f;
        config.TriggerbotVisibleCheck = 1;
        config.TriggerbotTeamCheck = 1;
        
        // Genel varsayılanlar
        config.OverlayEnabled = 1;
        config.MenuEnabled = 1;
        config.MenuKey = VK_INSERT;
        config.FPSLimit = 60;
        config.FOVColor = {1.0f, 1.0f, 1.0f, 0.5f};
        config.FOVCircleRadius = 90.0f;
        config.CurrentProfile = 2; // Custom
        
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        g_CurrentConfig = config;
        g_ConfigDirty = TRUE;
    }
    
    BOOL LoadConfig(const std::string& configPath) {
        g_ConfigPath = configPath;
        
        std::ifstream file(configPath);
        if (!file.is_open()) {
            // Dosya yok - varsayılanı oluştur
            CreateDefaultConfig();
            SaveConfig(configPath);
            return TRUE;
        }
        
        // JSON benzeri basit format kullan
        // Gerçek uygulamada: nlohmann/json veya rapidjson kullanılır
        // Basitleştirilmiş: Binary format kullan
        
        XALO_CONFIG config = {0};
        file.read(reinterpret_cast<char*>(&config), sizeof(config));
        file.close();
        
        if (file.gcount() != sizeof(config)) {
            CreateDefaultConfig();
            return TRUE;
        }
        
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        g_CurrentConfig = config;
        g_ConfigDirty = FALSE;
        
        return TRUE;
    }
    
    BOOL SaveConfig(const std::string& configPath) {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        
        std::ofstream file(configPath, std::ios::binary);
        if (!file.is_open()) {
            return FALSE;
        }
        
        file.write(reinterpret_cast<char*>(&g_CurrentConfig), sizeof(g_CurrentConfig));
        file.close();
        
        g_ConfigDirty = FALSE;
        return TRUE;
    }
    
    XALO_CONFIG GetConfig() {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        return g_CurrentConfig;
    }
    
    void UpdateConfig(const XALO_CONFIG& newConfig) {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        g_CurrentConfig = newConfig;
        g_ConfigDirty = TRUE;
    }
    
    void SetConfigValue(UINT32 offset, UINT32 value) {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        
        // Offset ile belirtilen alana değer yaz
        PUCHAR configBytes = reinterpret_cast<PUCHAR>(&g_CurrentConfig);
        
        if (offset < sizeof(g_CurrentConfig)) {
            *reinterpret_cast<PUINT32>(configBytes + offset) = value;
            g_ConfigDirty = TRUE;
        }
    }
    
    UINT32 GetConfigValue(UINT32 offset) {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        
        PUCHAR configBytes = reinterpret_cast<PUCHAR>(&g_CurrentConfig);
        
        if (offset < sizeof(g_CurrentConfig)) {
            return *reinterpret_cast<PUINT32>(configBytes + offset);
        }
        
        return 0;
    }
    
    void LoadRageProfile() {
        XALO_CONFIG config = GetConfig();
        
        // Rage ayarları - agresif aimbot
        config.AimbotEnabled = 1;
        config.AimbotTargetMode = 0; // Crosshair
        config.AimbotBoneTarget = 0; // Kafa
        config.AimbotSmoothingEnabled = 0; // Smoothing kapalı
        config.AimbotSmoothingFactor = 0.0f;
        config.AimbotFOV = 180.0f; // Geniş FOV
        config.AimbotRecoilControl = 0; // Recoil kontrol kapalı
        config.AimbotAutoTarget = 1;
        
        // ESP - maksimum bilgi
        config.ESPEnabled = 1;
        config.ESPBoxEnabled = 1;
        config.ESPHealthBarEnabled = 1;
        config.ESPNameEnabled = 1;
        config.ESPDistanceEnabled = 1;
        config.ESPHeadDotEnabled = 1;
        config.ESPLineEnabled = 1;
        config.ESPSnaplineEnabled = 1;
        config.ESPVisibleCheck = 0; // Görünürlük kontrolü kapalı
        config.ESPTeamCheck = 0; // Takım kontrolü kapalı
        
        // Triggerbot
        config.TriggerbotEnabled = 1;
        config.TriggerbotMinDelay = 10.0f;
        config.TriggerbotMaxDelay = 50.0f;
        
        config.CurrentProfile = 0;
        
        UpdateConfig(config);
    }
    
    void LoadLegitProfile() {
        XALO_CONFIG config = GetConfig();
        
        // Legit ayarları - insan benzeri
        config.AimbotEnabled = 1;
        config.AimbotTargetMode = 0;
        config.AimbotBoneTarget = 0;
        config.AimbotSmoothingEnabled = 1;
        config.AimbotSmoothingFactor = 15.0f; // Yüksek smoothing
        config.AimbotFOV = 30.0f; // Dar FOV
        config.AimbotRecoilControl = 1;
        config.AimbotRecoilStrength = 0.3f;
        config.AimbotAutoTarget = 0;
        
        // ESP - minimal
        config.ESPEnabled = 1;
        config.ESPBoxEnabled = 0;
        config.ESPHealthBarEnabled = 0;
        config.ESPNameEnabled = 0;
        config.ESPDistanceEnabled = 0;
        config.ESPHeadDotEnabled = 0;
        config.ESPLineEnabled = 0;
        config.ESPSnaplineEnabled = 0;
        config.ESPVisibleCheck = 1;
        config.ESPTeamCheck = 1;
        config.ESPEnemyOnly = 1;
        
        // Triggerbot - yavaş
        config.TriggerbotEnabled = 0;
        config.TriggerbotMinDelay = 100.0f;
        config.TriggerbotMaxDelay = 300.0f;
        
        config.CurrentProfile = 1;
        
        UpdateConfig(config);
    }
    
    void LoadCustomProfile() {
        // Custom profil - kullanıcı ayarları
        // Varsayılan değerlerle başla
        
        CreateDefaultConfig();
        g_CurrentConfig.CurrentProfile = 2;
    }
    
    void LoadProfile(UINT32 profileIndex) {
        switch (profileIndex) {
            case 0:
                LoadRageProfile();
                break;
            case 1:
                LoadLegitProfile();
                break;
            case 2:
                LoadCustomProfile();
                break;
            default:
                break;
        }
    }
    
    BOOL IsConfigDirty() {
        return g_ConfigDirty.load();
    }
    
    void ClearConfig() {
        std::lock_guard<std::mutex> lock(g_ConfigMutex);
        RtlZeroMemory(&g_CurrentConfig, sizeof(g_CurrentConfig));
        g_ConfigDirty = TRUE;
    }
}