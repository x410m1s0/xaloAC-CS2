// File: xaloAC/Overlay/OverlayWindow.h
#pragma once
// xaloAC - Overlay Pencere Başlık Dosyası

#include <windows.h>
#include <string>

namespace XaloOverlay {
    // Overlay pencere sınıfı
    class OverlayWindow {
    private:
        HWND m_WindowHandle;
        HWND m_TargetWindow;
        std::wstring m_WindowClassName;
        INT m_ScreenWidth;
        INT m_ScreenHeight;
        BOOL m_IsInitialized;
        
    public:
        OverlayWindow();
        ~OverlayWindow();
        
        // Pencereyi oluştur
        BOOL CreateOverlayWindow(const std::wstring& targetWindowTitle);
        
        // Pencereyi yok et
        void DestroyOverlayWindow();
        
        // Pencereyi CS2'ye hizala
        void AlignToTargetWindow();
        
        // Pencereyi göster/gizle
        void ShowOverlay();
        void HideOverlay();
        
        // Pencere handle'ını al
        HWND GetWindowHandle() const;
        
        // Hedef pencereyi bul
        HWND FindTargetWindow(const std::wstring& windowTitle);
        
        // Rastgele sınıf adı oluştur
        std::wstring GenerateRandomClassName();
        
        // Ekran boyutunu al
        void GetScreenSize(INT* OutWidth, INT* OutHeight);
        
        // Pencere prosedürü
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        
        // Overlay'i tespit edilmez yap
        void MakeOverlayUndetectable();
        
        // Click-through ayarla
        void SetClickThrough(BOOL enable);
    };
}