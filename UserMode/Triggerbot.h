// File: xaloAC/UserMode/Triggerbot.h
#pragma once
// xaloAC - Triggerbot Başlık Dosyası

#include <windows.h>
#include <random>
#include "../Shared/XaloShared.h"

namespace XaloTrigger {
    // Triggerbot sınıfı
    class Triggerbot {
    private:
        XALO_CONFIG m_Config;
        XALO_GAME_STATE m_GameState;
        BOOL m_TriggerbotActive;
        BOOL m_IsFiring;
        UINT64 m_LastFireTime;
        std::mt19937 m_RandomGen;
        
    public:
        Triggerbot();
        ~Triggerbot();
        
        // Triggerbot'u güncelle
        void Update(const XALO_GAME_STATE& gameState);
        
        // Triggerbot'u çalıştır
        void RunTriggerbot();
        
        // Crosshair altında düşman var mı?
        BOOL IsEnemyUnderCrosshair();
        
        // Tetikle
        void Fire();
        
        // Rastgele gecikme ekle
        void AddRandomDelay();
        
        // İnsan benzeri reaksiyon süresi
        DWORD GetReactionTime();
        
        // Konfigürasyonu ayarla
        void SetConfig(const XALO_CONFIG& config);
        
        // Triggerbot'u aç/kapat
        void ToggleTriggerbot();
        
        // Tetikleme tuşu basılı mı?
        BOOL IsTriggerKeyPressed();
    };
}