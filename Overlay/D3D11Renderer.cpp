// File: xaloAC/Overlay/D3D11Renderer.cpp
// xaloAC - DirectX 11 Renderer Uygulaması (Düzeltilmiş - Shader Derleme Eklendi)

#include "D3D11Renderer.h"
#include <chrono>
#include <d3dcompiler.h>
#include <vector>
#include <algorithm>

#pragma comment(lib, "d3dcompiler.lib")

namespace XaloOverlay {
    
    D3D11Renderer::D3D11Renderer()
        : m_WindowHandle(nullptr)
        , m_Device(nullptr)
        , m_DeviceContext(nullptr)
        , m_SwapChain(nullptr)
        , m_RenderTargetView(nullptr)
        , m_VertexShader(nullptr)
        , m_PixelShader(nullptr)
        , m_InputLayout(nullptr)
        , m_VertexBuffer(nullptr)
        , m_BlendState(nullptr)
        , m_ScreenWidth(1920)
        , m_ScreenHeight(1080)
        , m_Running(FALSE)
        , m_RenderReady(FALSE)
        , m_FPSLimit(60)
        , m_CurrentFPS(0) {
    }
    
    D3D11Renderer::~D3D11Renderer() {
        Cleanup();
    }
    
    BOOL D3D11Renderer::Initialize(HWND windowHandle, INT screenWidth, INT screenHeight) {
        m_WindowHandle = windowHandle;
        m_ScreenWidth = screenWidth;
        m_ScreenHeight = screenHeight;
        
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = screenWidth;
        swapChainDesc.BufferDesc.Height = screenHeight;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = windowHandle;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        
        D3D_FEATURE_LEVEL selectedFeatureLevel;
        
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_SwapChain,
            &m_Device,
            &selectedFeatureLevel,
            &m_DeviceContext
        );
        
        if (FAILED(hr)) {
            return FALSE;
        }
        
        ID3D11Texture2D* backBuffer = nullptr;
        hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        
        if (FAILED(hr)) {
            return FALSE;
        }
        
        hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
        backBuffer->Release();
        
        if (FAILED(hr)) {
            return FALSE;
        }
        
        if (!CreateShaders()) {
            return FALSE;
        }
        
        if (!CreateVertexBuffer()) {
            return FALSE;
        }
        
        if (!CreateBlendState()) {
            return FALSE;
        }
        
        m_RenderReady = TRUE;
        return TRUE;
    }
    
    void D3D11Renderer::Cleanup() {
        StopRenderLoop();
        
        if (m_BlendState) { m_BlendState->Release(); m_BlendState = nullptr; }
        if (m_VertexBuffer) { m_VertexBuffer->Release(); m_VertexBuffer = nullptr; }
        if (m_InputLayout) { m_InputLayout->Release(); m_InputLayout = nullptr; }
        if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
        if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
        if (m_RenderTargetView) { m_RenderTargetView->Release(); m_RenderTargetView = nullptr; }
        if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
        if (m_DeviceContext) { m_DeviceContext->Release(); m_DeviceContext = nullptr; }
        if (m_Device) { m_Device->Release(); m_Device = nullptr; }
        
        m_RenderReady = FALSE;
    }
    
    void D3D11Renderer::StartRenderLoop() {
        if (m_Running.load()) return;
        m_Running = TRUE;
        m_RenderThread = std::thread(&D3D11Renderer::RenderThreadFunc, this);
    }
    
    void D3D11Renderer::StopRenderLoop() {
        m_Running = FALSE;
        if (m_RenderThread.joinable()) {
            m_RenderThread.join();
        }
    }
    
    void D3D11Renderer::RenderThreadFunc() {
        auto frameStart = std::chrono::steady_clock::now();
        auto frameEnd = std::chrono::steady_clock::now();
        
        UINT frameCount = 0;
        auto fpsTimer = std::chrono::steady_clock::now();
        
        while (m_Running.load()) {
            frameStart = std::chrono::steady_clock::now();
            
            RenderFrame();
            
            frameCount++;
            auto currentTime = std::chrono::steady_clock::now();
            auto fpsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - fpsTimer).count();
            
            if (fpsElapsed >= 1000) {
                m_CurrentFPS = frameCount;
                frameCount = 0;
                fpsTimer = currentTime;
            }
            
            frameEnd = std::chrono::steady_clock::now();
            auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();
            
            if (m_FPSLimit > 0) {
                DWORD targetFrameTime = 1000 / m_FPSLimit;
                if (frameDuration < targetFrameTime) {
                    Sleep(static_cast<DWORD>(targetFrameTime - frameDuration));
                }
            }
        }
    }
    
    void D3D11Renderer::RenderFrame() {
        if (!m_RenderReady.load() || !m_DeviceContext || !m_RenderTargetView) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_RenderMutex);
        
        FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);
        m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, nullptr);
        
        D3D11_VIEWPORT viewport = {0};
        viewport.Width = static_cast<FLOAT>(m_ScreenWidth);
        viewport.Height = static_cast<FLOAT>(m_ScreenHeight);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_DeviceContext->RSSetViewports(1, &viewport);
        
        m_DeviceContext->OMSetBlendState(m_BlendState, nullptr, 0xFFFFFFFF);
        m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
        m_DeviceContext->IASetInputLayout(m_InputLayout);
        
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_DeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        m_SwapChain->Present(0, 0);
    }
    
    void D3D11Renderer::DrawLine(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT thickness) {
        FLOAT dx = x2 - x1;
        FLOAT dy = y2 - y1;
        FLOAT length = std::sqrt(dx * dx + dy * dy);
        
        if (length < 0.001f) return;
        
        FLOAT nx = -dy / length * thickness / 2.0f;
        FLOAT ny = dx / length * thickness / 2.0f;
        
        Vertex vertices[6];
        
        vertices[0].Position[0] = x1 + nx; vertices[0].Position[1] = y1 + ny;
        vertices[0].Color[0] = r; vertices[0].Color[1] = g; vertices[0].Color[2] = b; vertices[0].Color[3] = a;
        
        vertices[1].Position[0] = x1 - nx; vertices[1].Position[1] = y1 - ny;
        vertices[1].Color[0] = r; vertices[1].Color[1] = g; vertices[1].Color[2] = b; vertices[1].Color[3] = a;
        
        vertices[2].Position[0] = x2 + nx; vertices[2].Position[1] = y2 + ny;
        vertices[2].Color[0] = r; vertices[2].Color[1] = g; vertices[2].Color[2] = b; vertices[2].Color[3] = a;
        
        vertices[3].Position[0] = x2 + nx; vertices[3].Position[1] = y2 + ny;
        vertices[3].Color[0] = r; vertices[3].Color[1] = g; vertices[3].Color[2] = b; vertices[3].Color[3] = a;
        
        vertices[4].Position[0] = x1 - nx; vertices[4].Position[1] = y1 - ny;
        vertices[4].Color[0] = r; vertices[4].Color[1] = g; vertices[4].Color[2] = b; vertices[4].Color[3] = a;
        
        vertices[5].Position[0] = x2 - nx; vertices[5].Position[1] = y2 - ny;
        vertices[5].Color[0] = r; vertices[5].Color[1] = g; vertices[5].Color[2] = b; vertices[5].Color[3] = a;
        
        // Vertex buffer'a yaz
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(m_DeviceContext->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            m_DeviceContext->Unmap(m_VertexBuffer, 0);
            m_DeviceContext->Draw(6, 0);
        }
    }
    
    void D3D11Renderer::DrawRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT r, FLOAT g, FLOAT b, FLOAT a, BOOL filled, FLOAT thickness) {
        if (filled) {
            DrawFilledRect(x, y, width, height, r, g, b, a);
            return;
        }
        
        DrawLine(x, y, x + width, y, r, g, b, a, thickness);
        DrawLine(x + width, y, x + width, y + height, r, g, b, a, thickness);
        DrawLine(x + width, y + height, x, y + height, r, g, b, a, thickness);
        DrawLine(x, y + height, x, y, r, g, b, a, thickness);
    }
    
    void D3D11Renderer::DrawCircle(FLOAT centerX, FLOAT centerY, FLOAT radius, FLOAT r, FLOAT g, FLOAT b, FLOAT a, INT segments, FLOAT thickness) {
        if (segments < 8) segments = 8;
        
        FLOAT angleStep = 2.0f * 3.14159265f / segments;
        
        for (INT i = 0; i < segments; i++) {
            FLOAT angle1 = i * angleStep;
            FLOAT angle2 = (i + 1) * angleStep;
            
            FLOAT x1 = centerX + radius * std::cos(angle1);
            FLOAT y1 = centerY + radius * std::sin(angle1);
            FLOAT x2 = centerX + radius * std::cos(angle2);
            FLOAT y2 = centerY + radius * std::sin(angle2);
            
            DrawLine(x1, y1, x2, y2, r, g, b, a, thickness);
        }
    }
    
    void D3D11Renderer::DrawText(const wchar_t* text, FLOAT x, FLOAT y, FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT fontSize) {
        // DirectWrite ile metin çizimi
        // Basitleştirilmiş: Metin çizimi için DirectWrite kullanılır
        UNREFERENCED_PARAMETER(text);
        UNREFERENCED_PARAMETER(x);
        UNREFERENCED_PARAMETER(y);
        UNREFERENCED_PARAMETER(r);
        UNREFERENCED_PARAMETER(g);
        UNREFERENCED_PARAMETER(b);
        UNREFERENCED_PARAMETER(a);
        UNREFERENCED_PARAMETER(fontSize);
    }
    
    void D3D11Renderer::DrawFilledRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT r, FLOAT g, FLOAT b, FLOAT a) {
        Vertex vertices[6];
        
        vertices[0].Position[0] = x; vertices[0].Position[1] = y;
        vertices[0].Color[0] = r; vertices[0].Color[1] = g; vertices[0].Color[2] = b; vertices[0].Color[3] = a;
        
        vertices[1].Position[0] = x + width; vertices[1].Position[1] = y;
        vertices[1].Color[0] = r; vertices[1].Color[1] = g; vertices[1].Color[2] = b; vertices[1].Color[3] = a;
        
        vertices[2].Position[0] = x; vertices[2].Position[1] = y + height;
        vertices[2].Color[0] = r; vertices[2].Color[1] = g; vertices[2].Color[2] = b; vertices[2].Color[3] = a;
        
        vertices[3].Position[0] = x + width; vertices[3].Position[1] = y;
        vertices[3].Color[0] = r; vertices[3].Color[1] = g; vertices[3].Color[2] = b; vertices[3].Color[3] = a;
        
        vertices[4].Position[0] = x; vertices[4].Position[1] = y + height;
        vertices[4].Color[0] = r; vertices[4].Color[1] = g; vertices[4].Color[2] = b; vertices[4].Color[3] = a;
        
        vertices[5].Position[0] = x + width; vertices[5].Position[1] = y + height;
        vertices[5].Color[0] = r; vertices[5].Color[1] = g; vertices[5].Color[2] = b; vertices[5].Color[3] = a;
        
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(m_DeviceContext->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            m_DeviceContext->Unmap(m_VertexBuffer, 0);
            m_DeviceContext->Draw(6, 0);
        }
    }
    
    UINT D3D11Renderer::GetCurrentFPS() const {
        return m_CurrentFPS.load();
    }
    
    void D3D11Renderer::SetFPSLimit(UINT fpsLimit) {
        m_FPSLimit = fpsLimit;
    }
    
    void D3D11Renderer::SetVSync(BOOL enable) {
        UNREFERENCED_PARAMETER(enable);
    }
    
    BOOL D3D11Renderer::CreateShaders() {
        // Vertex shader kaynak kodu
        const char* vertexShaderSource = R"(
            struct VS_INPUT {
                float2 position : POSITION;
                float4 color : COLOR;
            };
            
            struct VS_OUTPUT {
                float4 position : SV_POSITION;
                float4 color : COLOR;
            };
            
            VS_OUTPUT main(VS_INPUT input) {
                VS_OUTPUT output;
                output.position = float4(input.position, 0.0f, 1.0f);
                output.color = input.color;
                return output;
            }
        )";
        
        // Pixel shader kaynak kodu
        const char* pixelShaderSource = R"(
            struct PS_INPUT {
                float4 position : SV_POSITION;
                float4 color : COLOR;
            };
            
            float4 main(PS_INPUT input) : SV_TARGET {
                return input.color;
            }
        )";
        
        // Vertex shader derle
        ID3DBlob* vertexShaderBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        
        HRESULT hr = D3DCompile(
            vertexShaderSource,
            strlen(vertexShaderSource),
            "VertexShader",
            nullptr,
            nullptr,
            "main",
            "vs_5_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3,
            0,
            &vertexShaderBlob,
            &errorBlob
        );
        
        if (FAILED(hr)) {
            if (errorBlob) errorBlob->Release();
            return FALSE;
        }
        
        hr = m_Device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            &m_VertexShader
        );
        
        if (FAILED(hr)) {
            vertexShaderBlob->Release();
            return FALSE;
        }
        
        // Input layout oluştur
        D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        
        hr = m_Device->CreateInputLayout(
            layoutDesc,
            ARRAYSIZE(layoutDesc),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            &m_InputLayout
        );
        
        vertexShaderBlob->Release();
        
        if (FAILED(hr)) {
            return FALSE;
        }
        
        // Pixel shader derle
        ID3DBlob* pixelShaderBlob = nullptr;
        
        hr = D3DCompile(
            pixelShaderSource,
            strlen(pixelShaderSource),
            "PixelShader",
            nullptr,
            nullptr,
            "main",
            "ps_5_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3,
            0,
            &pixelShaderBlob,
            &errorBlob
        );
        
        if (FAILED(hr)) {
            if (errorBlob) errorBlob->Release();
            return FALSE;
        }
        
        hr = m_Device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            &m_PixelShader
        );
        
        pixelShaderBlob->Release();
        
        return SUCCEEDED(hr);
    }
    
    BOOL D3D11Renderer::CreateVertexBuffer() {
        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(Vertex) * 10000;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
        return SUCCEEDED(m_Device->CreateBuffer(&bufferDesc, nullptr, &m_VertexBuffer));
    }
    
    BOOL D3D11Renderer::CreateBlendState() {
        D3D11_BLEND_DESC blendDesc = {0};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        
        return SUCCEEDED(m_Device->CreateBlendState(&blendDesc, &m_BlendState));
    }
    
    void D3D11Renderer::UpdateScreenSize(INT width, INT height) {
        m_ScreenWidth = width;
        m_ScreenHeight = height;
        
        if (m_SwapChain && m_RenderTargetView) {
            m_RenderTargetView->Release();
            m_RenderTargetView = nullptr;
            
            m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
            
            ID3D11Texture2D* backBuffer = nullptr;
            m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
            
            if (backBuffer) {
                m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
                backBuffer->Release();
            }
        }
    }
}