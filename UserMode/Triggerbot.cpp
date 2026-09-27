// File: xaloAC/UserMode/Triggerbot.cpp
// xaloAC - Triggerbot Uygulaması

#include "Triggerbot.h"
#include "XorStr.h"
#include <chrono>

namespace XaloTrigger {
    
    Triggerbot::Triggerbot()
        : m_TriggerbotActive(FALSE)
        , m_IsFiring(FALSE)
        , m_LastFireTime(0)
        , m_RandomGen(std::chrono::steady_clock::now().time_since_epoch().count()) {
        RtlZeroMemory(&m_Config, sizeof(m_Config));
        RtlZeroMemory(&m_GameState, sizeof(m_GameState));
    }
    
    Triggerbot::~Triggerbot() {
    }
    
    void Triggerbot::Update(const XALO_GAME_STATE& gameState) {
        m_GameState = gameState;
    }
    
    void Triggerbot::RunTriggerbot() {
        if (!m_Config.TriggerbotEnabled) {
            return;
        }
        
        // Tetikleme tuşu kontrolü
        if (!IsTriggerKeyPressed()) {
            m_IsFiring = FALSE;
            return;
        }
        
        // Crosshair altında düşman var mı?
        if (IsEnemyUnderCrosshair()) {
            Fire();
        }
    }
    
    BOOL Triggerbot::IsEnemyUnderCrosshair() {
        // Crosshair altındaki entity'yi kontrol et
        
        // Ekran merkezi
        FLOAT screenCenterX = 960.0f;
        FLOAT screenCenterY = 540.0f;
        
        // Crosshair yakınındaki oyuncuları kontrol et
        for (UINT32 i = 0; i < m_GameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = m_GameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            // Takım kontrolü
            if (m_Config.TriggerbotTeamCheck && player.Team == m_GameState.LocalPlayerTeam) {
                continue;
            }
            
            // Görünürlük kontrolü
            if (m_Config.TriggerbotVisibleCheck) {
                if (i < XALO_MAX_ENTITIES && m_GameState.VisibilityMap[i] != 1) {
                    continue;
                }
            }
            
            // Crosshair'e yakınlık kontrolü
            FLOAT dx = player.ScreenPosition[0] - screenCenterX;
            FLOAT dy = player.ScreenPosition[1] - screenCenterY;
            FLOAT distance = std::sqrt(dx * dx + dy * dy);
            
            // Crosshair yarıçapı (piksel cinsinden)
            FLOAT crosshairRadius = 5.0f;
            
            if (distance < crosshairRadius) {
                return TRUE;
            }
        }
        
        return FALSE;
    }
    
    void Triggerbot::Fire() {
        if (m_IsFiring) {
            return;
        }
        
        // Reaksiyon süresi
        DWORD reactionTime = GetReactionTime();
        
        // Son ateşleme zamanını kontrol et
        UINT64 currentTime = GetTickCount64();
        
        if (currentTime - m_LastFireTime < reactionTime) {
            return;
        }
        
        // Ateş et
        m_IsFiring = TRUE;
        m_LastFireTime = currentTime;
        
        // Fare tıklaması simüle et
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        
        // Kısa bir gecikme
        Sleep(10);
        
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        
        m_IsFiring = FALSE;
    }
    
    void Triggerbot::AddRandomDelay() {
        // Rastgele gecikme ekle (insan benzeri)
        std::uniform_int_distribution<DWORD> dist(
            static_cast<DWORD>(m_Config.TriggerbotMinDelay),
            static_cast<DWORD>(m_Config.TriggerbotMaxDelay)
        );
        
        DWORD delay = dist(m_RandomGen);
        Sleep(delay);
    }
    
    DWORD Triggerbot::GetReactionTime() {
        // İnsan reaksiyon süresi 150-300ms arası
        // Konfigüre edilebilir: 50-500ms
        std::uniform_int_distribution<DWORD> dist(
            static_cast<DWORD>(m_Config.TriggerbotMinDelay),
            static_cast<DWORD>(m_Config.TriggerbotMaxDelay)
        );
        
        return dist(m_RandomGen);
    }
    
    void Triggerbot::SetConfig(const XALO_CONFIG& config) {
        m_Config = config;
    }
    
    void Triggerbot::ToggleTriggerbot() {
        m_TriggerbotActive = !m_TriggerbotActive;
    }
    
    BOOL Triggerbot::IsTriggerKeyPressed() {
        // Tetikleme tuşu basılı mı?
        SHORT keyState = GetAsyncKeyState(m_Config.TriggerbotKey);
        
        // En yüksek bit basılı olduğunu gösterir
        return (keyState & 0x8000) != 0;
    }
}