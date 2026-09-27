// File: xaloAC/UserMode/Main.cpp
// xaloAC - Ana Giriş Noktası
// CS2 External Cheat - xaloAC v1.0

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include "ProcessManager.h"
#include "OffsetScanner.h"
#include "ESPRenderer.h"
#include "Aimbot.h"
#include "Triggerbot.h"
#include "ConfigManager.h"
#include "XorStr.h"
#include "AntiDebug.h"
#include "../Shared/XaloShared.h"
#include "../Shared/DriverLoader.h"
#include "../Overlay/OverlayWindow.h"
#include "../Overlay/D3D11Renderer.h"
#include "../Overlay/DrawFunctions.h"

// Global durum
static std::atomic<BOOL> g_Running = TRUE;
static std::atomic<BOOL> g_CS2Found = FALSE;
static std::atomic<BOOL> g_OverlayReady = FALSE;

// Global bileşenler
static XaloOverlay::OverlayWindow* g_OverlayWindow = nullptr;
static XaloOverlay::D3D11Renderer* g_D3D11Renderer = nullptr;
static XaloESP::ESPRenderer* g_ESPRenderer = nullptr;
static XaloAimbot::Aimbot* g_Aimbot = nullptr;
static XaloTrigger::Triggerbot* g_Triggerbot = nullptr;

// Global oyun durumu
static XALO_GAME_STATE g_GameState = {0};
static XALO_CONFIG g_Config = {0};

// Kernel sürücüsü handle'ı
static HANDLE g_DriverHandle = INVALID_HANDLE_VALUE;

// Shared memory
static PVOID g_SharedMemory = nullptr;

// Hata log dosyası
static FILE* g_ErrorLog = nullptr;

// Hata loglama
void LogError(const std::string& errorMessage) {
    if (!g_ErrorLog) {
        fopen_s(&g_ErrorLog, "xaloAC_error.log", "a");
    }
    
    if (g_ErrorLog) {
        fprintf(g_ErrorLog, "[%s] %s\n", __TIME__, errorMessage.c_str());
        fflush(g_ErrorLog);
    }
}

// Ctrl+C handler
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT || signal == CTRL_BREAK_EVENT) {
        g_Running = FALSE;
        return TRUE;
    }
    return FALSE;
}

// Sürücüyü başlat
BOOL InitializeDriver() {
    // Sürücüyü manuel olarak yükle
    std::wstring driverPath = L"C:\\xaloAC\\XaloACDriver.sys";
    
    if (!XaloDriverLoader::LoadDriverManually(driverPath)) {
        LogError(XaloDriverLoader::GetLastErrorMessage());
        return FALSE;
    }
    
    // Shared memory'yi haritala
    g_SharedMemory = XaloDriverLoader::MapSharedMemory();
    
    if (!g_SharedMemory) {
        LogError("Shared memory haritalanamadı.");
        return FALSE;
    }
    
    // Sürücü handle'ını al
    g_DriverHandle = CreateFileW(
        L"\\\\.\\XaloACDriver",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (g_DriverHandle == INVALID_HANDLE_VALUE) {
        LogError("Sürücü handle'ı açılamadı.");
        return FALSE;
    }
    
    return TRUE;
}

// Overlay'i başlat
BOOL InitializeOverlay() {
    g_OverlayWindow = new XaloOverlay::OverlayWindow();
    
    if (!g_OverlayWindow->CreateOverlayWindow(L"Counter-Strike 2")) {
        LogError("Overlay penceresi oluşturulamadı.");
        return FALSE;
    }
    
    INT screenWidth = 0;
    INT screenHeight = 0;
    g_OverlayWindow->GetScreenSize(&screenWidth, &screenHeight);
    
    g_D3D11Renderer = new XaloOverlay::D3D11Renderer();
    
    if (!g_D3D11Renderer->Initialize(g_OverlayWindow->GetWindowHandle(), screenWidth, screenHeight)) {
        LogError("D3D11 renderer başlatılamadı.");
        return FALSE;
    }
    
    // FPS limitini ayarla
    g_D3D11Renderer->SetFPSLimit(g_Config.FPSLimit);
    
    // Render döngüsünü başlat
    g_D3D11Renderer->StartRenderLoop();
    
    g_OverlayReady = TRUE;
    return TRUE;
}

// CS2'yi bekle
BOOL WaitForCS2() {
    DWORD processId = 0;
    
    if (!XaloProcess::WaitForCS2Process(&processId, 120000)) { // 2 dakika bekle
        LogError("CS2 başlatılmayı bekleniyor...");
        return FALSE;
    }
    
    g_GameState.Cs2ProcessId = processId;
    g_CS2Found = TRUE;
    
    return TRUE;
}

// Oyun durumunu güncelle
void UpdateGameState() {
    if (!g_CS2Found.load()) {
        return;
    }
    
    DWORD processId = static_cast<DWORD>(g_GameState.Cs2ProcessId);
    HANDLE processHandle = XaloProcess::OpenCS2Process(processId);
    
    if (!processHandle || processHandle == INVALID_HANDLE_VALUE) {
        return;
    }
    
    // Modül bilgilerini al
    XaloProcess::GetProcessInfo(processId, &g_GameState);
    
    // Offset'leri tara (ilk seferde veya değişiklik algılandığında)
    static BOOL offsetsScanned = FALSE;
    
    if (!offsetsScanned) {
        if (XaloOffset::ScanAllOffsets(processHandle, g_GameState.ClientBaseAddress, g_GameState.ClientSize, &g_GameState)) {
            offsetsScanned = TRUE;
        } else {
            // Yedek offset'lerle devam et
            offsetsScanned = TRUE;
        }
    }
    
    // View matrix'i oku
    if (g_GameState.OffsetViewMatrix != 0) {
        ULONG_PTR viewMatrixAddress = g_GameState.ClientBaseAddress + g_GameState.OffsetViewMatrix;
        
        // Kernel üzerinden oku
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            XaloProcess::ReadMemoryKernel(
                g_DriverHandle,
                processId,
                viewMatrixAddress,
                &g_GameState.ViewMatrix,
                sizeof(g_GameState.ViewMatrix)
            );
        } else {
            // User-mode fallback
            ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(viewMatrixAddress), &g_GameState.ViewMatrix, sizeof(g_GameState.ViewMatrix), nullptr);
        }
    }
    
    // Yerel oyuncu bilgisini oku
    if (g_GameState.OffsetLocalPlayerPawn != 0) {
        ULONG_PTR localPlayerAddress = g_GameState.ClientBaseAddress + g_GameState.OffsetLocalPlayerPawn;
        ULONG_PTR localPlayerPawn = 0;
        
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, localPlayerAddress, &localPlayerPawn, sizeof(localPlayerPawn));
        } else {
            ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(localPlayerAddress), &localPlayerPawn, sizeof(localPlayerPawn), nullptr);
        }
        
        g_GameState.LocalPlayerPawn = localPlayerPawn;
    }
    
    // Entity listesini oku
    if (g_GameState.OffsetEntityList != 0 && g_GameState.LocalPlayerPawn != 0) {
        ULONG_PTR entityListAddress = g_GameState.ClientBaseAddress + g_GameState.OffsetEntityList;
        ULONG_PTR entityList = 0;
        
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, entityListAddress, &entityList, sizeof(entityList));
        } else {
            ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(entityListAddress), &entityList, sizeof(entityList), nullptr);
        }
        
        // Entity listesini oku
        // Basitleştirilmiş: İlk 64 entity'yi oku
        UINT32 entityCount = min(64, XALO_MAX_ENTITIES);
        g_GameState.EntityCount = entityCount;
        
        for (UINT32 i = 0; i < entityCount; i++) {
            ULONG_PTR entityAddress = 0;
            ULONG_PTR entityEntry = entityList + i * 8;
            
            if (g_DriverHandle != INVALID_HANDLE_VALUE) {
                XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, entityEntry, &entityAddress, sizeof(entityAddress));
            } else {
                ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(entityEntry), &entityAddress, sizeof(entityAddress), nullptr);
            }
            
            if (entityAddress == 0) {
                continue;
            }
            
            // Entity bilgisini oku
            XALO_ENTITY_INFO entityInfo = {0};
            entityInfo.PawnAddress = entityAddress;
            
            // Sağlık
            UINT32 health = 0;
            ULONG_PTR healthAddress = entityAddress + 0x32C;
            
            if (g_DriverHandle != INVALID_HANDLE_VALUE) {
                XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, healthAddress, &health, sizeof(health));
            } else {
                ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(healthAddress), &health, sizeof(health), nullptr);
            }
            entityInfo.Health = health;
            
            // Takım
            UINT32 team = 0;
            ULONG_PTR teamAddress = entityAddress + 0x3BF;
            
            if (g_DriverHandle != INVALID_HANDLE_VALUE) {
                XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, teamAddress, &team, sizeof(team));
            } else {
                ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(teamAddress), &team, sizeof(team), nullptr);
            }
            entityInfo.Team = team;
            
            // Pozisyon
            FLOAT position[3] = {0};
            ULONG_PTR positionAddress = entityAddress + 0x1200;
            
            if (g_DriverHandle != INVALID_HANDLE_VALUE) {
                XaloProcess::ReadMemoryKernel(g_DriverHandle, processId, positionAddress, position, sizeof(position));
            } else {
                ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(positionAddress), position, sizeof(position), nullptr);
            }
            
            entityInfo.Position[0] = position[0];
            entityInfo.Position[1] = position[1];
            entityInfo.Position[2] = position[2];
            entityInfo.IsAlive = (health > 0 && health <= 100) ? 1 : 0;
            
            g_GameState.Entities[i] = entityInfo;
        }
        
        // Yerel oyuncu takımını bul
        for (UINT32 i = 0; i < entityCount; i++) {
            if (g_GameState.Entities[i].PawnAddress == g_GameState.LocalPlayerPawn) {
                g_GameState.LocalPlayerTeam = g_GameState.Entities[i].Team;
                g_GameState.LocalPlayerHealth = g_GameState.Entities[i].Health;
                break;
            }
        }
        
        // Oyuncu bilgilerini oluştur
        UINT32 playerCount = 0;
        
        for (UINT32 i = 0; i < entityCount && playerCount < XALO_MAX_PLAYERS; i++) {
            if (g_GameState.Entities[i].IsAlive == 1 && g_GameState.Entities[i].Health > 0) {
                XALO_PLAYER_INFO playerInfo = {0};
                playerInfo.PawnAddress = g_GameState.Entities[i].PawnAddress;
                playerInfo.Health = g_GameState.Entities[i].Health;
                playerInfo.Team = g_GameState.Entities[i].Team;
                playerInfo.IsAlive = 1;
                playerInfo.Position[0] = g_GameState.Entities[i].Position[0];
                playerInfo.Position[1] = g_GameState.Entities[i].Position[1];
                playerInfo.Position[2] = g_GameState.Entities[i].Position[2];
                playerInfo.ViewOffsetZ = g_GameState.Entities[i].ViewOffsetZ;
                
                // Ekran pozisyonunu hesapla
                FLOAT screenPos[2] = {0};
                if (XaloWorldToScreen(playerInfo.Position, g_GameState.ViewMatrix, screenPos, 1920, 1080)) {
                    playerInfo.ScreenPosition[0] = screenPos[0];
                    playerInfo.ScreenPosition[1] = screenPos[1];
                }
                
                // Kafa pozisyonu
                FLOAT headPos[3] = {playerInfo.Position[0], playerInfo.Position[1], playerInfo.Position[2] + 72.0f};
                FLOAT headScreen[2] = {0};
                if (XaloWorldToScreen(headPos, g_GameState.ViewMatrix, headScreen, 1920, 1080)) {
                    playerInfo.ScreenHeadPosition[0] = headScreen[0];
                    playerInfo.ScreenHeadPosition[1] = headScreen[1];
                }
                
                // Ayak pozisyonu
                FLOAT footScreen[2] = {0};
                if (XaloWorldToScreen(playerInfo.Position, g_GameState.ViewMatrix, footScreen, 1920, 1080)) {
                    playerInfo.ScreenFootPosition[0] = footScreen[0];
                    playerInfo.ScreenFootPosition[1] = footScreen[1];
                }
                
                // Mesafe hesapla
                if (g_GameState.EntityCount > 0) {
                    FLOAT dx = playerInfo.Position[0] - g_GameState.Entities[0].Position[0];
                    FLOAT dy = playerInfo.Position[1] - g_GameState.Entities[0].Position[1];
                    FLOAT dz = playerInfo.Position[2] - g_GameState.Entities[0].Position[2];
                    playerInfo.Distance = sqrt(dx * dx + dy * dy + dz * dz) * 0.01905f; // Unit'ten metreye
                }
                
                g_GameState.Players[playerCount++] = playerInfo;
            }
        }
        
        g_GameState.PlayerCount = playerCount;
    }
    
    XaloProcess::CloseCS2Process(processHandle);
}

// Ana render fonksiyonu
void RenderLoop() {
    while (g_Running.load()) {
        if (!g_OverlayReady.load()) {
            Sleep(100);
            continue;
        }
        
        // Oyun durumunu güncelle
        UpdateGameState();
        
        // ESP'yi güncelle
        if (g_ESPRenderer) {
            g_ESPRenderer->Update(g_GameState);
        }
        
        // Aimbot'u güncelle
        if (g_Aimbot) {
            g_Aimbot->Update(g_GameState);
        }
        
        // Triggerbot'u güncelle
        if (g_Triggerbot) {
            g_Triggerbot->Update(g_GameState);
        }
        
        Sleep(1); // 1ms bekle - CPU kullanımını azalt
    }
}

// Aimbot döngüsü
void AimbotLoop() {
    while (g_Running.load()) {
        if (g_Aimbot && g_CS2Found.load()) {
            g_Aimbot->RunAimbot();
        }
        
        Sleep(1);
    }
}

// Triggerbot döngüsü
void TriggerbotLoop() {
    while (g_Running.load()) {
        if (g_Triggerbot && g_CS2Found.load()) {
            g_Triggerbot->RunTriggerbot();
        }
        
        Sleep(1);
    }
}

// Menü tuşu kontrolü
void MenuKeyLoop() {
    static BOOL menuOpen = FALSE;
    
    while (g_Running.load()) {
        if (GetAsyncKeyState(g_Config.MenuKey) & 0x0001) {
            menuOpen = !menuOpen;
            g_Config.MenuEnabled = menuOpen ? 1 : 0;
        }
        
        Sleep(50);
    }
}

// Ana fonksiyon
int main() {
    // Konsol başlığı
    SetConsoleTitleW(L"xaloAC - CS2 External Cheat");
    
    // Ctrl+C handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    
    // Anti-debug kontrolü
    XaloAntiDebug::InitializeAntiDebug();
    
    // Konfigürasyonu yükle
    if (!XaloConfig::LoadConfig("xaloAC_config.bin")) {
        XaloConfig::CreateDefaultConfig();
    }
    
    g_Config = XaloConfig::GetConfig();
    
    // Sürücüyü başlat
    if (!InitializeDriver()) {
        LogError("Sürücü başlatılamadı. User-mode fallback kullanılacak.");
    }
    
    // CS2'yi bekle
    if (!WaitForCS2()) {
        LogError("CS2 bulunamadı. Program kapatılıyor.");
        
        // Temizlik
        if (g_DriverHandle != INVALID_HANDLE_VALUE) {
            XaloDriverLoader::UnloadDriver();
        }
        
        return 1;
    }
    
    // Overlay'i başlat
    if (!InitializeOverlay()) {
        LogError("Overlay başlatılamadı. GDI fallback kullanılacak.");
    }
    
    // ESP renderer'ı oluştur
    g_ESPRenderer = new XaloESP::ESPRenderer();
    g_ESPRenderer->SetConfig(g_Config);
    g_ESPRenderer->SetScreenSize(1920, 1080);
    
    // Aimbot'u oluştur
    g_Aimbot = new XaloAimbot::Aimbot();
    g_Aimbot->SetConfig(g_Config);
    
    // Triggerbot'u oluştur
    g_Triggerbot = new XaloTrigger::Triggerbot();
    g_Triggerbot->SetConfig(g_Config);
    
    // Render döngüsü
    std::thread renderThread(RenderLoop);
    std::thread aimbotThread(AimbotLoop);
    std::thread triggerbotThread(TriggerbotLoop);
    std::thread menuKeyThread(MenuKeyLoop);
    
    // Ana döngü
    while (g_Running.load()) {
        // Konfigürasyon değişikliklerini kontrol et
        if (XaloConfig::IsConfigDirty()) {
            g_Config = XaloConfig::GetConfig();
            
            if (g_ESPRenderer) {
                g_ESPRenderer->SetConfig(g_Config);
            }
            
            if (g_Aimbot) {
                g_Aimbot->SetConfig(g_Config);
            }
            
            if (g_Triggerbot) {
                g_Triggerbot->SetConfig(g_Config);
            }
            
            if (g_D3D11Renderer) {
                g_D3D11Renderer->SetFPSLimit(g_Config.FPSLimit);
            }
        }
        
        // CS2 hala çalışıyor mu?
        DWORD processId = 0;
        if (!XaloProcess::FindCS2Process(&processId)) {
            g_CS2Found = FALSE;
        } else {
            g_CS2Found = TRUE;
            g_GameState.Cs2ProcessId = processId;
        }
        
        Sleep(100);
    }
    
    // Thread'leri bekle
    renderThread.join();
    aimbotThread.join();
    triggerbotThread.join();
    menuKeyThread.join();
    
    // Temizlik
    if (g_Triggerbot) {
        delete g_Triggerbot;
        g_Triggerbot = nullptr;
    }
    
    if (g_Aimbot) {
        delete g_Aimbot;
        g_Aimbot = nullptr;
    }
    
    if (g_ESPRenderer) {
        delete g_ESPRenderer;
        g_ESPRenderer = nullptr;
    }
    
    if (g_D3D11Renderer) {
        delete g_D3D11Renderer;
        g_D3D11Renderer = nullptr;
    }
    
    if (g_OverlayWindow) {
        delete g_OverlayWindow;
        g_OverlayWindow = nullptr;
    }
    
    // Sürücüyü kaldır
    if (g_DriverHandle != INVALID_HANDLE_VALUE) {
        XaloDriverLoader::UnloadDriver();
        CloseHandle(g_DriverHandle);
        g_DriverHandle = INVALID_HANDLE_VALUE;
    }
    
    // Konfigürasyonu kaydet
    XaloConfig::SaveConfig("xaloAC_config.bin");
    
    // Hata logunu kapat
    if (g_ErrorLog) {
        fclose(g_ErrorLog);
        g_ErrorLog = nullptr;
    }
    
    return 0;
}