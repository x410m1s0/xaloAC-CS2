// File: xaloAC/UserMode/ESPRenderer.cpp
// xaloAC - ESP Renderer Uygulaması

#include "ESPRenderer.h"
#include "XorStr.h"
#include <cmath>
#include <algorithm>

namespace XaloESP {
    
    ESPRenderer::ESPRenderer() 
        : m_ScreenWidth(1920)
        , m_ScreenHeight(1080) {
        m_Players.clear();
        RtlZeroMemory(&m_GameState, sizeof(m_GameState));
        RtlZeroMemory(&m_Config, sizeof(m_Config));
    }
    
    ESPRenderer::~ESPRenderer() {
        m_Players.clear();
    }
    
    void ESPRenderer::Update(const XALO_GAME_STATE& gameState) {
        m_GameState = gameState;
        
        // Oyuncuları güncelle
        m_Players.clear();
        
        for (UINT32 i = 0; i < gameState.PlayerCount; i++) {
            ESPPlayerData playerData;
            playerData.PlayerInfo = gameState.Players[i];
            playerData.IsValid = (playerData.PlayerInfo.IsAlive == 1);
            
            // Düşman kontrolü
            playerData.IsEnemy = (playerData.PlayerInfo.Team != gameState.LocalPlayerTeam);
            
            // Görünürlük kontrolü
            if (i < XALO_MAX_ENTITIES) {
                playerData.IsVisible = (gameState.VisibilityMap[i] == 1);
            } else {
                playerData.IsVisible = FALSE;
            }
            
            if (playerData.IsValid) {
                m_Players.push_back(playerData);
            }
        }
        
        // Oyuncuları filtrele
        FilterPlayers();
    }
    
    void ESPRenderer::FilterPlayers() {
        // Takım kontrolü
        if (m_Config.ESPTeamCheck) {
            m_Players.erase(
                std::remove_if(m_Players.begin(), m_Players.end(),
                    [this](const ESPPlayerData& player) {
                        return !player.IsEnemy;
                    }),
                m_Players.end()
            );
        }
        
        // Sadece düşmanlar
        if (m_Config.ESPEnemyOnly) {
            m_Players.erase(
                std::remove_if(m_Players.begin(), m_Players.end(),
                    [](const ESPPlayerData& player) {
                        return !player.IsEnemy;
                    }),
                m_Players.end()
            );
        }
        
        // Görünürlük kontrolü
        if (m_Config.ESPVisibleCheck) {
            m_Players.erase(
                std::remove_if(m_Players.begin(), m_Players.end(),
                    [](const ESPPlayerData& player) {
                        return !player.IsVisible;
                    }),
                m_Players.end()
            );
        }
        
        // Mesafe kontrolü
        if (m_Config.ESPMaxDistance > 0) {
            m_Players.erase(
                std::remove_if(m_Players.begin(), m_Players.end(),
                    [this](const ESPPlayerData& player) {
                        return player.PlayerInfo.Distance > m_Config.ESPMaxDistance;
                    }),
                m_Players.end()
            );
        }
    }
    
    void ESPRenderer::Render(void* renderer) {
        if (!renderer || !m_Config.ESPEnabled) {
            return;
        }
        
        // Her oyuncuyu çiz
        for (const auto& player : m_Players) {
            if (m_Config.ESPBoxEnabled) {
                DrawBoxESP(renderer, player);
            }
            
            if (m_Config.ESPHealthBarEnabled) {
                DrawHealthBar(renderer, player);
            }
            
            if (m_Config.ESPNameEnabled) {
                DrawName(renderer, player);
            }
            
            if (m_Config.ESPDistanceEnabled) {
                DrawDistance(renderer, player);
            }
            
            if (m_Config.ESPHeadDotEnabled) {
                DrawHeadDot(renderer, player);
            }
            
            if (m_Config.ESPLineEnabled) {
                DrawLineESP(renderer, player);
            }
            
            if (m_Config.ESPSnaplineEnabled) {
                DrawSnapline(renderer, player);
            }
        }
    }
    
    void ESPRenderer::DrawBoxESP(void* renderer, const ESPPlayerData& player) {
        // Box ESP çizimi
        // Oyuncunun ekran pozisyonunu kullanarak dikdörtgen çiz
        
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenY = player.PlayerInfo.ScreenPosition[1];
        FLOAT screenHeadX = player.PlayerInfo.ScreenHeadPosition[0];
        FLOAT screenHeadY = player.PlayerInfo.ScreenHeadPosition[1];
        FLOAT screenFootX = player.PlayerInfo.ScreenFootPosition[0];
        FLOAT screenFootY = player.PlayerInfo.ScreenFootPosition[1];
        
        // Kutu boyutlarını hesapla
        FLOAT boxHeight = screenFootY - screenHeadY;
        FLOAT boxWidth = boxHeight * 0.6f; // Genişlik oranı
        
        FLOAT boxLeft = screenX - boxWidth / 2;
        FLOAT boxRight = screenX + boxWidth / 2;
        FLOAT boxTop = screenHeadY;
        FLOAT boxBottom = screenFootY;
        
        // Renk seç
        XALO_COLOR boxColor = player.IsVisible ? m_Config.ESPVisibleColor : m_Config.ESPHiddenColor;
        
        // Çizim çağrıları (overlay renderer'a iletilir)
        // Bu fonksiyonlar DrawFunctions üzerinden çağrılır
    }
    
    void ESPRenderer::DrawHealthBar(void* renderer, const ESPPlayerData& player) {
        // Health bar çizimi
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenHeadY = player.PlayerInfo.ScreenHeadPosition[1];
        FLOAT screenFootY = player.PlayerInfo.ScreenFootPosition[1];
        
        // Bar boyutları
        FLOAT barWidth = 4.0f;
        FLOAT barHeight = screenFootY - screenHeadY;
        FLOAT barX = screenX - barWidth - 10.0f;
        FLOAT barY = screenHeadY;
        
        // Sağlık yüzdesi
        FLOAT healthPercent = static_cast<FLOAT>(player.PlayerInfo.Health) / 100.0f;
        
        // Renk gradienti: yeşil -> sarı -> kırmızı
        XALO_COLOR healthColor;
        if (healthPercent > 0.5f) {
            // Yeşil -> sarı
            healthColor.R = 2.0f * (1.0f - healthPercent);
            healthColor.G = 1.0f;
            healthColor.B = 0.0f;
        } else {
            // Sarı -> kırmızı
            healthColor.R = 1.0f;
            healthColor.G = 2.0f * healthPercent;
            healthColor.B = 0.0f;
        }
        healthColor.A = 1.0f;
        
        // Bar çizimi
    }
    
    void ESPRenderer::DrawName(void* renderer, const ESPPlayerData& player) {
        // Oyuncu adı çizimi
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenHeadY = player.PlayerInfo.ScreenHeadPosition[1];
        
        // İsim pozisyonu (kutunun üstünde)
        FLOAT textX = screenX;
        FLOAT textY = screenHeadY - 15.0f;
        
        // İsim metni
        std::string playerName = player.PlayerInfo.PlayerName;
        
        // Metin çizimi
    }
    
    void ESPRenderer::DrawDistance(void* renderer, const ESPPlayerData& player) {
        // Mesafe göstergesi
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenFootY = player.PlayerInfo.ScreenFootPosition[1];
        
        // Mesafe metni
        char distanceText[32];
        sprintf_s(distanceText, "%.1fm", player.PlayerInfo.Distance);
        
        FLOAT textX = screenX;
        FLOAT textY = screenFootY + 5.0f;
        
        // Metin çizimi
    }
    
    void ESPRenderer::DrawHeadDot(void* renderer, const ESPPlayerData& player) {
        // Head dot çizimi
        FLOAT screenHeadX = player.PlayerInfo.ScreenHeadPosition[0];
        FLOAT screenHeadY = player.PlayerInfo.ScreenHeadPosition[1];
        
        // Nokta boyutu
        FLOAT dotRadius = 3.0f;
        
        // Renk
        XALO_COLOR dotColor = m_Config.ESPHeadDotColor;
        
        // Daire çizimi
    }
    
    void ESPRenderer::DrawLineESP(void* renderer, const ESPPlayerData& player) {
        // Line ESP - oyuncuya doğru çizgi
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenY = player.PlayerInfo.ScreenPosition[1];
        
        // Ekranın altından oyuncuya çizgi
        FLOAT startX = static_cast<FLOAT>(m_ScreenWidth) / 2.0f;
        FLOAT startY = static_cast<FLOAT>(m_ScreenHeight);
        
        // Renk
        XALO_COLOR lineColor = m_Config.ESPLineColor;
        
        // Çizgi çizimi
    }
    
    void ESPRenderer::DrawSnapline(void* renderer, const ESPPlayerData& player) {
        // Snapline - oyuncunun altına dikey çizgi
        FLOAT screenX = player.PlayerInfo.ScreenPosition[0];
        FLOAT screenFootY = player.PlayerInfo.ScreenFootPosition[1];
        
        // Ekranın altından oyuncunun ayaklarına dikey çizgi
        FLOAT startX = screenX;
        FLOAT startY = static_cast<FLOAT>(m_ScreenHeight);
        FLOAT endX = screenX;
        FLOAT endY = screenFootY;
        
        // Renk
        XALO_COLOR snaplineColor = m_Config.ESPLineColor;
        
        // Çizgi çizimi
    }
    
    BOOL ESPRenderer::IsPlayerVisible(const XALO_PLAYER_INFO& player) {
        // BSP parsing ile görünürlük analizi
        // Bu, basitleştirilmiş bir kontrol
        // Gerçek uygulamada: BSP tree traversal yapılır
        
        // Basitleştirilmiş: IsSpotted bayrağını kontrol et
        if (player.IsSpotted) {
            return TRUE;
        }
        
        // Line of sight kontrolü
        // Oyuncu ile yerel oyuncu arasında duvar var mı?
        
        return FALSE;
    }
    
    void ESPRenderer::SetScreenSize(INT width, INT height) {
        m_ScreenWidth = width;
        m_ScreenHeight = height;
    }
    
    void ESPRenderer::SetConfig(const XALO_CONFIG& config) {
        m_Config = config;
    }
}