// File: xaloAC/Overlay/D3D11Renderer.h
#pragma once
// xaloAC - DirectX 11 Renderer Başlık Dosyası (Düzeltilmiş - Eksik Include Eklendi)

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <atomic>
#include <thread>
#include <mutex>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace XaloOverlay {
    struct Vertex {
        FLOAT Position[2];
        FLOAT Color[4];
    };
    
    class D3D11Renderer {
    private:
        HWND m_WindowHandle;
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;
        IDXGISwapChain* m_SwapChain;
        ID3D11RenderTargetView* m_RenderTargetView;
        ID3D11VertexShader* m_VertexShader;
        ID3D11PixelShader* m_PixelShader;
        ID3D11InputLayout* m_InputLayout;
        ID3D11Buffer* m_VertexBuffer;
        ID3D11BlendState* m_BlendState;
        
        INT m_ScreenWidth;
        INT m_ScreenHeight;
        
        std::thread m_RenderThread;
        std::atomic<BOOL> m_Running;
        std::atomic<BOOL> m_RenderReady;
        std::mutex m_RenderMutex;
        
        UINT m_FPSLimit;
        std::atomic<UINT> m_CurrentFPS;
        
    public:
        D3D11Renderer();
        ~D3D11Renderer();
        
        BOOL Initialize(HWND windowHandle, INT screenWidth, INT screenHeight);
        void Cleanup();
        void StartRenderLoop();
        void StopRenderLoop();
        void RenderFrame();
        
        void DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT thickness = 1.0f);
        void DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT r, FLOAT g, FLOAT b, FLOAT a, BOOL filled = FALSE, FLOAT thickness = 1.0f);
        void DrawCircle(FLOAT centerX, FLOAT centerY, FLOAT radius, FLOAT r, FLOAT g, FLOAT b, FLOAT a, INT segments = 32, FLOAT thickness = 1.0f);
        void DrawText(const wchar_t* text, FLOAT x, FLOAT y, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT fontSize = 14.0f);
        void DrawFilledRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT r, FLOAT g, FLOAT b, FLOAT a);
        
        UINT GetCurrentFPS() const;
        void SetFPSLimit(UINT fpsLimit);
        void SetVSync(BOOL enable);
        
        void RenderThreadFunc();
        BOOL CreateShaders();
        BOOL CreateVertexBuffer();
        BOOL CreateBlendState();
        void UpdateScreenSize(INT width, INT height);
    };
}