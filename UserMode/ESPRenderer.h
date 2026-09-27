// File: xaloAC/UserMode/ESPRenderer.h
#pragma once
// xaloAC - ESP Renderer Başlık Dosyası

#include <windows.h>
#include <vector>
#include "../Shared/XaloShared.h"

// Forward declaration
namespace XaloOverlay {
    class D3D11Renderer;
}

namespace XaloESP {
    // ESP oyuncu bilgisi
    struct ESPPlayerData {
        XALO_PLAYER_INFO PlayerInfo;
        BOOL IsVisible;
        BOOL IsEnemy;
        BOOL IsValid;
    };
    
    // ESP renderer sınıfı
    class ESPRenderer {
    private:
        std::vector<ESPPlayerData> m_Players;
        XALO_GAME_STATE m_GameState;
        XALO_CONFIG m_Config;
        INT m_ScreenWidth;
        INT m_ScreenHeight;
        
    public:
        ESPRenderer();
        ~ESPRenderer();
        
        // ESP'yi güncelle
        void Update(const XALO_GAME_STATE& gameState);
        
        // ESP'yi çiz
        void Render(void* renderer);
        
        // Oyuncuları filtrele
        void FilterPlayers();
        
        // Box ESP çiz
        void DrawBoxESP(void* renderer, const ESPPlayerData& player);
        
        // Health bar çiz
        void DrawHealthBar(void* renderer, const ESPPlayerData& player);
        
        // İsim çiz
        void DrawName(void* renderer, const ESPPlayerData& player);
        
        // Mesafe çiz
        void DrawDistance(void* renderer, const ESPPlayerData& player);
        
        // Head dot çiz
        void DrawHeadDot(void* renderer, const ESPPlayerData& player);
        
        // Line ESP çiz
        void DrawLineESP(void* renderer, const ESPPlayerData& player);
        
        // Snapline çiz
        void DrawSnapline(void* renderer, const ESPPlayerData& player);
        
        // Ekran boyutunu ayarla
        void SetScreenSize(INT width, INT height);
        
        // Konfigürasyonu ayarla
        void SetConfig(const XALO_CONFIG& config);
        
        // Görünürlük kontrolü (BSP parsing)
        BOOL IsPlayerVisible(const XALO_PLAYER_INFO& player);
    };
}