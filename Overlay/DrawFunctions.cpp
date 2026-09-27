// File: xaloAC/Overlay/DrawFunctions.cpp
// xaloAC - Çizim Fonksiyonları Uygulaması

#include "DrawFunctions.h"
#include "D3D11Renderer.h"
#include <cmath>
#include <algorithm>
#include <cstdarg>
#include <cstdio>

namespace XaloDraw {
    
    void DrawBox(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, XALO_COLOR color, FLOAT thickness) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        d3dRenderer->DrawRect(x, y, width, height, color.R, color.G, color.B, color.A, FALSE, thickness);
    }
    
    void DrawFilledBox(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, XALO_COLOR color) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        d3dRenderer->DrawFilledRect(x, y, width, height, color.R, color.G, color.B, color.A);
    }
    
    void DrawLine(void* renderer, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, XALO_COLOR color, FLOAT thickness) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        d3dRenderer->DrawLine(x1, y1, x2, y2, color.R, color.G, color.B, color.A, thickness);
    }
    
    void DrawText(void* renderer, const std::string& text, FLOAT x, FLOAT y, XALO_COLOR color, FLOAT fontSize) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        // String'i wide string'e dönüştür
        std::wstring wideText(text.begin(), text.end());
        
        d3dRenderer->DrawText(wideText.c_str(), x, y, color.R, color.G, color.B, color.A, fontSize);
    }
    
    void DrawCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color, INT segments, FLOAT thickness) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        d3dRenderer->DrawCircle(centerX, centerY, radius, color.R, color.G, color.B, color.A, segments, thickness);
    }
    
    void DrawFilledCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color, INT segments) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        // Dolu daire - üçgen fan ile çizilir
        // Basitleştirilmiş: Çok sayıda çizgi ile doldur
        
        if (segments < 8) segments = 8;
        
        FLOAT angleStep = 2.0f * 3.14159265f / segments;
        
        for (INT i = 0; i < segments; i++) {
            FLOAT angle = i * angleStep;
            FLOAT x = centerX + radius * std::cos(angle);
            FLOAT y = centerY + radius * std::sin(angle);
            
            d3dRenderer->DrawLine(centerX, centerY, x, y, color.R, color.G, color.B, color.A, 1.0f);
        }
    }
    
    void DrawHealthBar(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, INT health, XALO_COLOR color) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        // Arka plan
        XALO_COLOR bgColor = MakeColor(0.0f, 0.0f, 0.0f, 0.5f);
        d3dRenderer->DrawFilledRect(x - 1, y - 1, width + 2, height + 2, bgColor.R, bgColor.G, bgColor.B, bgColor.A);
        
        // Sağlık yüzdesi
        FLOAT healthPercent = static_cast<FLOAT>(health) / 100.0f;
        FLOAT fillHeight = height * healthPercent;
        
        // Renk gradienti
        XALO_COLOR healthColor = GetHealthColor(health);
        
        // Dolu bar
        d3dRenderer->DrawFilledRect(x, y + (height - fillHeight), width, fillHeight, healthColor.R, healthColor.G, healthColor.B, healthColor.A);
    }
    
    void DrawFOVCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        d3dRenderer->DrawCircle(centerX, centerY, radius, color.R, color.G, color.B, color.A, 64, 1.0f);
    }
    
    void DrawCrosshair(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT size, XALO_COLOR color) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        // Crosshair çizimi - 4 çizgi
        FLOAT halfSize = size / 2.0f;
        
        // Üst
        d3dRenderer->DrawLine(centerX, centerY - halfSize, centerX, centerY - halfSize - 5, color.R, color.G, color.B, color.A, 1.5f);
        // Alt
        d3dRenderer->DrawLine(centerX, centerY + halfSize, centerX, centerY + halfSize + 5, color.R, color.G, color.B, color.A, 1.5f);
        // Sol
        d3dRenderer->DrawLine(centerX - halfSize, centerY, centerX - halfSize - 5, centerY, color.R, color.G, color.B, color.A, 1.5f);
        // Sağ
        d3dRenderer->DrawLine(centerX + halfSize, centerY, centerX + halfSize + 5, centerY, color.R, color.G, color.B, color.A, 1.5f);
    }
    
    void DrawRadar(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, const XALO_GAME_STATE& gameState, const XALO_CONFIG& config) {
        auto d3dRenderer = static_cast<XaloOverlay::D3D11Renderer*>(renderer);
        if (!d3dRenderer) return;
        
        // Radar arka planı
        XALO_COLOR bgColor = MakeColor(0.0f, 0.0f, 0.0f, 0.3f);
        DrawFilledCircle(renderer, centerX, centerY, radius, bgColor);
        
        // Radar çerçevesi
        XALO_COLOR borderColor = MakeColor(1.0f, 1.0f, 1.0f, 0.5f);
        DrawCircle(renderer, centerX, centerY, radius, borderColor, 64, 1.0f);
        
        // Oyuncuları radar üzerinde göster
        for (UINT32 i = 0; i < gameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = gameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            // Takım kontrolü
            if (config.ESPTeamCheck && player.Team == gameState.LocalPlayerTeam) {
                continue;
            }
            
            // Oyuncunun radar üzerindeki konumu
            FLOAT dx = player.Position[0] - gameState.Entities[0].Position[0];
            FLOAT dy = player.Position[1] - gameState.Entities[0].Position[1];
            
            // Radar ölçeği (1 birim = X piksel)
            FLOAT radarScale = radius / 50.0f; // 50 metrelik yarıçap
            
            FLOAT radarX = centerX + dx * radarScale;
            FLOAT radarY = centerY + dy * radarScale;
            
            // Radar sınırları içinde mi?
            if (radarX < centerX - radius || radarX > centerX + radius ||
                radarY < centerY - radius || radarY > centerY + radius) {
                continue;
            }
            
            // Oyuncuyu nokta olarak çiz
            XALO_COLOR dotColor;
            if (player.Team == gameState.LocalPlayerTeam) {
                dotColor = MakeColor(0.0f, 1.0f, 0.0f, 1.0f); // Takım arkadaşı - yeşil
            } else {
                dotColor = MakeColor(1.0f, 0.0f, 0.0f, 1.0f); // Düşman - kırmızı
            }
            
            DrawFilledCircle(renderer, radarX, radarY, 3.0f, dotColor);
        }
    }
    
    XALO_COLOR MakeColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a) {
        XALO_COLOR color;
        color.R = r;
        color.G = g;
        color.B = b;
        color.A = a;
        return color;
    }
    
    XALO_COLOR LerpColor(const XALO_COLOR& color1, const XALO_COLOR& color2, FLOAT t) {
        XALO_COLOR result;
        result.R = color1.R + (color2.R - color1.R) * t;
        result.G = color1.G + (color2.G - color1.G) * t;
        result.B = color1.B + (color2.B - color1.B) * t;
        result.A = color1.A + (color2.A - color1.A) * t;
        return result;
    }
    
    XALO_COLOR GetHealthColor(INT health) {
        FLOAT healthPercent = static_cast<FLOAT>(health) / 100.0f;
        
        XALO_COLOR green = MakeColor(0.0f, 1.0f, 0.0f, 1.0f);
        XALO_COLOR yellow = MakeColor(1.0f, 1.0f, 0.0f, 1.0f);
        XALO_COLOR red = MakeColor(1.0f, 0.0f, 0.0f, 1.0f);
        
        if (healthPercent > 0.5f) {
            // Yeşil -> Sarı (0.5 -> 1.0 arası)
            FLOAT t = (healthPercent - 0.5f) * 2.0f;
            return LerpColor(yellow, green, t);
        } else {
            // Kırmızı -> Sarı (0.0 -> 0.5 arası)
            FLOAT t = healthPercent * 2.0f;
            return LerpColor(red, yellow, t);
        }
    }
}