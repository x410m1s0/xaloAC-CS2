// File: xaloAC/Overlay/DrawFunctions.h
#pragma once
// xaloAC - Çizim Fonksiyonları Başlık Dosyası

#include <windows.h>
#include <string>
#include "../Shared/XaloShared.h"

// Forward declaration
namespace XaloOverlay {
    class D3D11Renderer;
}

namespace XaloDraw {
    // Çizim fonksiyonları
    void DrawBox(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, XALO_COLOR color, FLOAT thickness = 1.5f);
    
    void DrawFilledBox(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, XALO_COLOR color);
    
    void DrawLine(void* renderer, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, XALO_COLOR color, FLOAT thickness = 1.0f);
    
    void DrawText(void* renderer, const std::string& text, FLOAT x, FLOAT y, XALO_COLOR color, FLOAT fontSize = 14.0f);
    
    void DrawCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color, INT segments = 32, FLOAT thickness = 1.0f);
    
    void DrawFilledCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color, INT segments = 32);
    
    void DrawHealthBar(void* renderer, FLOAT x, FLOAT y, FLOAT width, FLOAT height, INT health, XALO_COLOR color);
    
    void DrawFOVCircle(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, XALO_COLOR color);
    
    void DrawCrosshair(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT size, XALO_COLOR color);
    
    void DrawRadar(void* renderer, FLOAT centerX, FLOAT centerY, FLOAT radius, const XALO_GAME_STATE& gameState, const XALO_CONFIG& config);
    
    // Renk dönüşümü
    XALO_COLOR MakeColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f);
    
    // Gradient renk
    XALO_COLOR LerpColor(const XALO_COLOR& color1, const XALO_COLOR& color2, FLOAT t);
    
    // Sağlık rengi (yeşil -> sarı -> kırmızı)
    XALO_COLOR GetHealthColor(INT health);
}