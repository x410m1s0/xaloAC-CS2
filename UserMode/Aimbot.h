// File: xaloAC/UserMode/Aimbot.h
#pragma once
// xaloAC - Aimbot Başlık Dosyası

#include <windows.h>
#include <vector>
#include <cmath>
#include "../Shared/XaloShared.h"

namespace XaloAimbot {
    // Hedef seçim modları
    enum TargetMode {
        TARGET_CROSSHAIR = 0,   // En yakın crosshair
        TARGET_LOWEST_HEALTH = 1, // En düşük sağlık
        TARGET_CLOSEST = 2,     // En yakın mesafe
        TARGET_HIGHEST_THREAT = 3 // En yüksek tehdit
    };
    
    // Kemik hedefleri
    enum BoneTarget {
        BONE_HEAD = 0,
        BONE_NECK = 1,
        BONE_CHEST = 2,
        BONE_STOMACH = 3,
        BONE_PELVIS = 4,
        BONE_RANDOM = 5
    };
    
    // Aimbot sınıfı
    class Aimbot {
    private:
        XALO_CONFIG m_Config;
        XALO_GAME_STATE m_GameState;
        BOOL m_AimbotActive;
        UINT32 m_CurrentTarget;
        FLOAT m_LastAimAngle[2]; // Pitch, Yaw
        
    public:
        Aimbot();
        ~Aimbot();
        
        // Aimbot'u güncelle
        void Update(const XALO_GAME_STATE& gameState);
        
        // Aimbot'u çalıştır
        void RunAimbot();
        
        // Hedef seç
        UINT32 SelectTarget();
        
        // Crosshair'e en yakın hedefi bul
        UINT32 FindClosestToCrosshair();
        
        // En düşük sağlıklı hedefi bul
        UINT32 FindLowestHealth();
        
        // En yakın hedefi bul
        UINT32 FindClosestTarget();
        
        // En yüksek tehditli hedefi bul
        UINT32 FindHighestThreat();
        
        // Hedefe kilitlen
        void AimAtTarget(UINT32 targetIndex);
        
        // Aim açısını hesapla
        void CalculateAimAngle(const FLOAT targetPos[3], FLOAT* OutPitch, FLOAT* OutYaw);
        
        // Smoothing uygula (Bezier curve)
        void ApplyBezierSmoothing(FLOAT currentAngle[2], FLOAT targetAngle[2], FLOAT* OutSmoothedAngle);
        
        // Recoil kontrolü
        void ApplyRecoilControl(FLOAT aimAngle[2], FLOAT* OutAdjustedAngle);
        
        // Rastgele sapma ekle
        void AddRandomDeviation(FLOAT aimAngle[2]);
        
        // Hedef kemik pozisyonunu al
        BOOL GetTargetBonePosition(UINT32 targetIndex, UINT32 boneTarget, FLOAT* OutBonePosition);
        
        // Görünürlük kontrolü
        BOOL IsTargetVisible(UINT32 targetIndex);
        
        // Takım kontrolü
        BOOL IsTargetEnemy(UINT32 targetIndex);
        
        // Aimbot'u aç/kapat
        void ToggleAimbot();
        
        // Aimbot durumunu al
        BOOL IsAimbotActive() const;
        
        // Konfigürasyonu ayarla
        void SetConfig(const XALO_CONFIG& config);
        
        // FOV içinde mi kontrol et
        BOOL IsTargetInFOV(UINT32 targetIndex);
        
        // Mesafe hesapla
        FLOAT CalculateDistance(const FLOAT pos1[3], const FLOAT pos2[3]);
        
        // Açı normalleştir
        void NormalizeAngle(FLOAT* angle);
    };
}