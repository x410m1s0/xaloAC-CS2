// File: xaloAC/Overlay/OverlayWindow.cpp
// xaloAC - Overlay Pencere Uygulaması

#include "OverlayWindow.h"
#include "../UserMode/XorStr.h"
#include <random>
#include <chrono>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace XaloOverlay {
    
    OverlayWindow::OverlayWindow()
        : m_WindowHandle(nullptr)
        , m_TargetWindow(nullptr)
        , m_ScreenWidth(1920)
        , m_ScreenHeight(1080)
        , m_IsInitialized(FALSE) {
    }
    
    OverlayWindow::~OverlayWindow() {
        DestroyOverlayWindow();
    }
    
    BOOL OverlayWindow::CreateOverlayWindow(const std::wstring& targetWindowTitle) {
        // Hedef pencereyi bul
        m_TargetWindow = FindTargetWindow(targetWindowTitle);
        
        if (!m_TargetWindow) {
            return FALSE;
        }
        
        // Rastgele sınıf adı oluştur
        m_WindowClassName = GenerateRandomClassName();
        
        // Pencere sınıfını kaydet
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = m_WindowClassName.c_str();
        wc.hIconSm = nullptr;
        
        if (!RegisterClassExW(&wc)) {
            return FALSE;
        }
        
        // Ekran boyutunu al
        GetScreenSize(&m_ScreenWidth, &m_ScreenHeight);
        
        // Overlay penceresini oluştur
        m_WindowHandle = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            m_WindowClassName.c_str(),
            L"",
            WS_POPUP,
            0, 0,
            m_ScreenWidth, m_ScreenHeight,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr
        );
        
        if (!m_WindowHandle) {
            UnregisterClassW(m_WindowClassName.c_str(), GetModuleHandleW(nullptr));
            return FALSE;
        }
        
        // Pencereyi şeffaf yap
        SetLayeredWindowAttributes(m_WindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);
        
        // Click-through ayarla
        SetClickThrough(TRUE);
        
        // Ekran görüntüsü koruması
        SetWindowDisplayAffinity(m_WindowHandle, WDA_EXCLUDEFROMCAPTURE);
        
        // Overlay'i tespit edilmez yap
        MakeOverlayUndetectable();
        
        // Pencereyi CS2'ye hizala
        AlignToTargetWindow();
        
        // Pencereyi göster
        ShowOverlay();
        
        m_IsInitialized = TRUE;
        return TRUE;
    }
    
    void OverlayWindow::DestroyOverlayWindow() {
        if (m_WindowHandle) {
            DestroyWindow(m_WindowHandle);
            m_WindowHandle = nullptr;
        }
        
        if (!m_WindowClassName.empty()) {
            UnregisterClassW(m_WindowClassName.c_str(), GetModuleHandleW(nullptr));
            m_WindowClassName.clear();
        }
        
        m_IsInitialized = FALSE;
    }
    
    void OverlayWindow::AlignToTargetWindow() {
        if (!m_TargetWindow || !m_WindowHandle) {
            return;
        }
        
        RECT targetRect;
        if (!GetWindowRect(m_TargetWindow, &targetRect)) {
            return;
        }
        
        // CS2 penceresinin konumuna ve boyutuna hizala
        SetWindowPos(
            m_WindowHandle,
            HWND_TOPMOST,
            targetRect.left,
            targetRect.top,
            targetRect.right - targetRect.left,
            targetRect.bottom - targetRect.top,
            SWP_SHOWWINDOW | SWP_NOACTIVATE
        );
        
        m_ScreenWidth = targetRect.right - targetRect.left;
        m_ScreenHeight = targetRect.bottom - targetRect.top;
    }
    
    void OverlayWindow::ShowOverlay() {
        if (m_WindowHandle) {
            ShowWindow(m_WindowHandle, SW_SHOWNOACTIVATE);
            SetWindowPos(m_WindowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    
    void OverlayWindow::HideOverlay() {
        if (m_WindowHandle) {
            ShowWindow(m_WindowHandle, SW_HIDE);
        }
    }
    
    HWND OverlayWindow::GetWindowHandle() const {
        return m_WindowHandle;
    }
    
    HWND OverlayWindow::FindTargetWindow(const std::wstring& windowTitle) {
        // CS2 penceresini ara
        return FindWindowW(nullptr, windowTitle.c_str());
    }
    
    std::wstring OverlayWindow::GenerateRandomClassName() {
        // Rastgele sınıf adı oluştur
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 35);
        
        const wchar_t chars[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::wstring className = L"";
        
        // 16 karakterlik rastgele isim
        for (int i = 0; i < 16; i++) {
            className += chars[dist(gen)];
        }
        
        return className;
    }
    
    void OverlayWindow::GetScreenSize(INT* OutWidth, INT* OutHeight) {
        if (OutWidth) {
            *OutWidth = GetSystemMetrics(SM_CXSCREEN);
        }
        
        if (OutHeight) {
            *OutHeight = GetSystemMetrics(SM_CYSCREEN);
        }
    }
    
    LRESULT CALLBACK OverlayWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            
            case WM_PAINT:
                // DirectX tarafından render edilir
                return 0;
            
            default:
                return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    
    void OverlayWindow::MakeOverlayUndetectable() {
        if (!m_WindowHandle) {
            return;
        }
        
        // Pencereyi VAC'den gizle
        // 1. Pencere adını gizle
        SetWindowTextW(m_WindowHandle, L"");
        
        // 2. Pencereyi tool window yap
        LONG_PTR exStyle = GetWindowLongPtrW(m_WindowHandle, GWL_EXSTYLE);
        exStyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtrW(m_WindowHandle, GWL_EXSTYLE, exStyle);
        
        // 3. Pencereyi taskbar'dan gizle
        LONG_PTR style = GetWindowLongPtrW(m_WindowHandle, GWL_STYLE);
        style &= ~WS_EX_APPWINDOW;
        SetWindowLongPtrW(m_WindowHandle, GWL_STYLE, style);
        
        // 4. DWM cloaking (pencereyi compositor'dan gizle)
        BOOL cloak = TRUE;
        DwmSetWindowAttribute(m_WindowHandle, DWMWA_CLOAK, &cloak, sizeof(cloak));
    }
    
    void OverlayWindow::SetClickThrough(BOOL enable) {
        if (!m_WindowHandle) {
            return;
        }
        
        LONG_PTR exStyle = GetWindowLongPtrW(m_WindowHandle, GWL_EXSTYLE);
        
        if (enable) {
            exStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED;
        } else {
            exStyle &= ~WS_EX_TRANSPARENT;
        }
        
        SetWindowLongPtrW(m_WindowHandle, GWL_EXSTYLE, exStyle);
    }
}