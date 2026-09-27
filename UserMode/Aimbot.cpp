// File: xaloAC/UserMode/Aimbot.cpp
// xaloAC - Aimbot Uygulaması

#include "Aimbot.h"
#include "XorStr.h"
#include <algorithm>
#include <random>

namespace XaloAimbot {
    
    Aimbot::Aimbot()
        : m_AimbotActive(FALSE)
        , m_CurrentTarget(0)
        , m_LastAimAngle{0.0f, 0.0f} {
        RtlZeroMemory(&m_Config, sizeof(m_Config));
        RtlZeroMemory(&m_GameState, sizeof(m_GameState));
    }
    
    Aimbot::~Aimbot() {
    }
    
    void Aimbot::Update(const XALO_GAME_STATE& gameState) {
        m_GameState = gameState;
    }
    
    void Aimbot::RunAimbot() {
        if (!m_Config.AimbotEnabled) {
            return;
        }
        
        // Hedef seç
        UINT32 targetIndex = SelectTarget();
        
        if (targetIndex == UINT32_MAX) {
            return;
        }
        
        m_CurrentTarget = targetIndex;
        
        // Hedefe kilitlen
        AimAtTarget(targetIndex);
    }
    
    UINT32 Aimbot::SelectTarget() {
        if (!m_Config.AimbotEnabled) {
            return UINT32_MAX;
        }
        
        switch (m_Config.AimbotTargetMode) {
            case TARGET_CROSSHAIR:
                return FindClosestToCrosshair();
            case TARGET_LOWEST_HEALTH:
                return FindLowestHealth();
            case TARGET_CLOSEST:
                return FindClosestTarget();
            case TARGET_HIGHEST_THREAT:
                return FindHighestThreat();
            default:
                return FindClosestToCrosshair();
        }
    }
    
    UINT32 Aimbot::FindClosestToCrosshair() {
        FLOAT bestDistance = FLT_MAX;
        UINT32 bestTarget = UINT32_MAX;
        
        FLOAT screenCenterX = 960.0f; // Varsayılan ekran merkezi
        FLOAT screenCenterY = 540.0f;
        
        for (UINT32 i = 0; i < m_GameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = m_GameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            // Takım kontrolü
            if (m_Config.AimbotTeamCheck && player.Team == m_GameState.LocalPlayerTeam) {
                continue;
            }
            
            // Görünürlük kontrolü
            if (m_Config.AimbotVisibleCheck && !IsTargetVisible(i)) {
                continue;
            }
            
            // FOV kontrolü
            if (!IsTargetInFOV(i)) {
                continue;
            }
            
            // Crosshair'e uzaklığı hesapla
            FLOAT dx = player.ScreenPosition[0] - screenCenterX;
            FLOAT dy = player.ScreenPosition[1] - screenCenterY;
            FLOAT distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = i;
            }
        }
        
        return bestTarget;
    }
    
    UINT32 Aimbot::FindLowestHealth() {
        UINT32 lowestHealth = UINT32_MAX;
        UINT32 bestTarget = UINT32_MAX;
        
        for (UINT32 i = 0; i < m_GameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = m_GameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            if (m_Config.AimbotTeamCheck && player.Team == m_GameState.LocalPlayerTeam) {
                continue;
            }
            
            if (m_Config.AimbotVisibleCheck && !IsTargetVisible(i)) {
                continue;
            }
            
            if (player.Health < lowestHealth) {
                lowestHealth = player.Health;
                bestTarget = i;
            }
        }
        
        return bestTarget;
    }
    
    UINT32 Aimbot::FindClosestTarget() {
        FLOAT closestDistance = FLT_MAX;
        UINT32 bestTarget = UINT32_MAX;
        
        for (UINT32 i = 0; i < m_GameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = m_GameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            if (m_Config.AimbotTeamCheck && player.Team == m_GameState.LocalPlayerTeam) {
                continue;
            }
            
            if (m_Config.AimbotVisibleCheck && !IsTargetVisible(i)) {
                continue;
            }
            
            if (player.Distance < closestDistance) {
                closestDistance = player.Distance;
                bestTarget = i;
            }
        }
        
        return bestTarget;
    }
    
    UINT32 Aimbot::FindHighestThreat() {
        FLOAT highestThreat = -FLT_MAX;
        UINT32 bestTarget = UINT32_MAX;
        
        for (UINT32 i = 0; i < m_GameState.PlayerCount; i++) {
            const XALO_PLAYER_INFO& player = m_GameState.Players[i];
            
            if (player.IsAlive != 1) continue;
            
            if (m_Config.AimbotTeamCheck && player.Team == m_GameState.LocalPlayerTeam) {
                continue;
            }
            
            // Tehdit hesaplama: Yakınlık + sağlık + bize bakıyor mu?
            FLOAT threat = 0.0f;
            
            // Yakınlık tehdidi
            threat += (100.0f - player.Distance) * 2.0f;
            
            // Sağlık tehdidi (düşük sağlık daha az tehdit)
            threat += (100.0f - player.Health) * 0.5f;
            
            // IsSpotted - bizi görüyor mu?
            if (player.IsSpotted) {
                threat += 50.0f;
            }
            
            if (threat > highestThreat) {
                highestThreat = threat;
                bestTarget = i;
            }
        }
        
        return bestTarget;
    }
    
    void Aimbot::AimAtTarget(UINT32 targetIndex) {
        if (targetIndex >= m_GameState.PlayerCount) {
            return;
        }
        
        const XALO_PLAYER_INFO& player = m_GameState.Players[targetIndex];
        
        // Hedef kemik pozisyonunu al
        FLOAT bonePosition[3] = {0};
        UINT32 boneTarget = m_Config.AimbotBoneTarget;
        
        // Random bone seçimi
        if (boneTarget == BONE_RANDOM) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<UINT32> dist(0, 4); // 5 ana kemik
            boneTarget = dist(gen);
        }
        
        if (!GetTargetBonePosition(targetIndex, boneTarget, bonePosition)) {
            return;
        }
        
        // Aim açısını hesapla
        FLOAT targetPitch = 0.0f;
        FLOAT targetYaw = 0.0f;
        CalculateAimAngle(bonePosition, &targetPitch, &targetYaw);
        
        FLOAT targetAngle[2] = {targetPitch, targetYaw};
        FLOAT smoothedAngle[2] = {targetPitch, targetYaw};
        
        // Smoothing uygula
        if (m_Config.AimbotSmoothingEnabled) {
            ApplyBezierSmoothing(m_LastAimAngle, targetAngle, smoothedAngle);
        }
        
        // Recoil kontrolü
        if (m_Config.AimbotRecoilControl) {
            ApplyRecoilControl(smoothedAngle, smoothedAngle);
        }
        
        // Rastgele sapma
        AddRandomDeviation(smoothedAngle);
        
        // Açıyı uygula (view angles yaz)
        // Bu, kernel sürücüsü üzerinden yapılır
        
        // Açıyı kaydet
        m_LastAimAngle[0] = smoothedAngle[0];
        m_LastAimAngle[1] = smoothedAngle[1];
        
        // View angles'ı yaz
        // Yerel oyuncunun view angles offset'ine yazılır
        ULONG_PTR viewAnglesAddress = m_GameState.LocalPlayerPawn + 0x1A0; // m_angEyeAngles
        // Kernel üzerinden yazma yapılır
    }
    
    void Aimbot::CalculateAimAngle(const FLOAT targetPos[3], FLOAT* OutPitch, FLOAT* OutYaw) {
        if (!OutPitch || !OutYaw) {
            return;
        }
        
        // Yerel oyuncu pozisyonu
        FLOAT localPos[3] = {0};
        // Local player pozisyonu GameState'ten alınır
        // Basitleştirilmiş: Entities[0] yerel oyuncu
        
        if (m_GameState.EntityCount > 0) {
            localPos[0] = m_GameState.Entities[0].Position[0];
            localPos[1] = m_GameState.Entities[0].Position[1];
            localPos[2] = m_GameState.Entities[0].Position[2] + m_GameState.Entities[0].ViewOffsetZ;
        }
        
        // Delta hesapla
        FLOAT deltaX = targetPos[0] - localPos[0];
        FLOAT deltaY = targetPos[1] - localPos[1];
        FLOAT deltaZ = targetPos[2] - localPos[2];
        
        // Yatay mesafe
        FLOAT horizontalDistance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        
        // Pitch (yukarı/aşağı)
        *OutPitch = std::atan2(-deltaZ, horizontalDistance) * (180.0f / 3.14159265f);
        
        // Yaw (sol/sağ)
        *OutYaw = std::atan2(deltaY, deltaX) * (180.0f / 3.14159265f);
        
        // Normalize
        NormalizeAngle(OutPitch);
        NormalizeAngle(OutYaw);
    }
    
    void Aimbot::ApplyBezierSmoothing(FLOAT currentAngle[2], FLOAT targetAngle[2], FLOAT* OutSmoothedAngle) {
        if (!OutSmoothedAngle) {
            return;
        }
        
        // Bezier curve tabanlı yumuşatma
        // Kontrol noktaları: P0 = current, P1 = ara nokta, P2 = target
        
        FLOAT smoothFactor = m_Config.AimbotSmoothingFactor;
        
        // Smoothing faktörü 0-20 arası (20 = çok yavaş)
        FLOAT t = 1.0f / (smoothFactor + 1.0f);
        
        // Bezier interpolation
        for (int i = 0; i < 2; i++) {
            // P0 = current, P1 = midpoint, P2 = target
            FLOAT p0 = currentAngle[i];
            FLOAT p2 = targetAngle[i];
            FLOAT p1 = (p0 + p2) / 2.0f;
            
            // Quadratic Bezier: B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
            FLOAT oneMinusT = 1.0f - t;
            OutSmoothedAngle[i] = oneMinusT * oneMinusT * p0 + 2.0f * oneMinusT * t * p1 + t * t * p2;
        }
    }
    
    void Aimbot::ApplyRecoilControl(FLOAT aimAngle[2], FLOAT* OutAdjustedAngle) {
        if (!OutAdjustedAngle) {
            return;
        }
        
        // Recoil kontrolü
        // Silah geri tepmesini telafi et
        
        FLOAT recoilStrength = m_Config.AimbotRecoilStrength;
        
        // Basitleştirilmiş: Pitch'e hafif yukarı telafi ekle
        OutAdjustedAngle[0] = aimAngle[0] + recoilStrength * 0.5f;
        OutAdjustedAngle[1] = aimAngle[1];
    }
    
    void Aimbot::AddRandomDeviation(FLOAT aimAngle[2]) {
        // İnsan benzeri rastgele sapma
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<FLOAT> dist(-0.1f, 0.1f);
        
        // Çok küçük sapmalar ekle
        aimAngle[0] += dist(gen) * 0.01f;
        aimAngle[1] += dist(gen) * 0.01f;
    }
    
    BOOL Aimbot::GetTargetBonePosition(UINT32 targetIndex, UINT32 boneTarget, FLOAT* OutBonePosition) {
        if (targetIndex >= m_GameState.PlayerCount || !OutBonePosition) {
            return FALSE;
        }
        
        const XALO_PLAYER_INFO& player = m_GameState.Players[targetIndex];
        
        // Kemik indeksi
        UINT32 boneIndex = 0;
        
        switch (boneTarget) {
            case BONE_HEAD: boneIndex = 6; break;    // head_0
            case BONE_NECK: boneIndex = 5; break;    // neck_0
            case BONE_CHEST: boneIndex = 4; break;   // chest
            case BONE_STOMACH: boneIndex = 3; break; // belly
            case BONE_PELVIS: boneIndex = 1; break;  // pelvis
            default: boneIndex = 6; break;
        }
        
        if (boneIndex >= XALO_MAX_BONES) {
            return FALSE;
        }
        
        // Kemik pozisyonunu kopyala
        OutBonePosition[0] = player.BonePositions[boneIndex][0];
        OutBonePosition[1] = player.BonePositions[boneIndex][1];
        OutBonePosition[2] = player.BonePositions[boneIndex][2];
        
        // Kemik pozisyonu sıfır değilse geçerli
        return (OutBonePosition[0] != 0.0f || OutBonePosition[1] != 0.0f || OutBonePosition[2] != 0.0f);
    }
    
    BOOL Aimbot::IsTargetVisible(UINT32 targetIndex) {
        if (targetIndex >= XALO_MAX_ENTITIES) {
            return FALSE;
        }
        
        return (m_GameState.VisibilityMap[targetIndex] == 1);
    }
    
    BOOL Aimbot::IsTargetEnemy(UINT32 targetIndex) {
        if (targetIndex >= m_GameState.PlayerCount) {
            return FALSE;
        }
        
        return (m_GameState.Players[targetIndex].Team != m_GameState.LocalPlayerTeam);
    }
    
    void Aimbot::ToggleAimbot() {
        m_AimbotActive = !m_AimbotActive;
    }
    
    BOOL Aimbot::IsAimbotActive() const {
        return m_AimbotActive;
    }
    
    void Aimbot::SetConfig(const XALO_CONFIG& config) {
        m_Config = config;
    }
    
    BOOL Aimbot::IsTargetInFOV(UINT32 targetIndex) {
        if (targetIndex >= m_GameState.PlayerCount) {
            return FALSE;
        }
        
        const XALO_PLAYER_INFO& player = m_GameState.Players[targetIndex];
        
        // Ekran merkezi
        FLOAT screenCenterX = 960.0f;
        FLOAT screenCenterY = 540.0f;
        
        // FOV içinde mi?
        FLOAT dx = player.ScreenPosition[0] - screenCenterX;
        FLOAT dy = player.ScreenPosition[1] - screenCenterY;
        FLOAT distance = std::sqrt(dx * dx + dy * dy);
        
        return distance <= m_Config.AimbotFOV;
    }
    
    FLOAT Aimbot::CalculateDistance(const FLOAT pos1[3], const FLOAT pos2[3]) {
        FLOAT dx = pos1[0] - pos2[0];
        FLOAT dy = pos1[1] - pos2[1];
        FLOAT dz = pos1[2] - pos2[2];
        
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    void Aimbot::NormalizeAngle(FLOAT* angle) {
        if (!angle) {
            return;
        }
        
        // Yaw'ı -180 ile 180 arasına normalize et
        while (*angle > 180.0f) {
            *angle -= 360.0f;
        }
        
        while (*angle < -180.0f) {
            *angle += 360.0f;
        }
        
        // Pitch'i -89 ile 89 arasına sınırla
        if (*angle > 89.0f) {
            *angle = 89.0f;
        }
        
        if (*angle < -89.0f) {
            *angle = -89.0f;
        }
    }
}