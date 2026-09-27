// File: xaloAC/Shared/XaloShared.h
#pragma once
// xaloAC - Paylaşılan Yapılar ve Tanımlamalar
// Bu başlık dosyası hem kernel sürücüsü hem de user-mode uygulama tarafından ortak kullanılır.

#include <windows.h>

// ==================== TEMEL SABİTLER ====================

#define XALO_SHARED_MEMORY_SIZE     0x200000    // 2MB shared memory bölgesi
#define XALO_MAX_ENTITIES           256         // Maksimum entity sayısı
#define XALO_MAX_PLAYERS            64          // Maksimum oyuncu sayısı
#define XALO_MAX_BONES              256         // Maksimum kemik sayısı
#define XALO_MAX_MODULES            64          // Maksimum modül sayısı
#define XALO_RING_BUFFER_SIZE       0x10000     // 64KB ring buffer
#define XALO_MAX_PLAYER_NAME        128         // Maksimum oyuncu adı uzunluğu
#define XALO_DRIVER_TAG             'xAC'       // Sürücü bellek etiketi (4CC)

// IOCTL kodları - Kernel ve UserMode arasında eşleşmeli
#define XALO_IOCTL_GET_SHARED_MEMORY    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_GET_XOR_KEY          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_GET_MODULE_BASE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_READ_MEMORY          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_WRITE_MEMORY         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_GET_PROCESS_ID       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define XALO_IOCTL_SHUTDOWN             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

// ==================== İLETİŞİM KOMUTLARI ====================

typedef enum _XALO_COMMAND {
    XALO_CMD_NONE = 0,
    XALO_CMD_READ_MEMORY = 1,
    XALO_CMD_WRITE_MEMORY = 2,
    XALO_CMD_GET_MODULE_BASE = 3,
    XALO_CMD_GET_PROCESS_ID = 4,
    XALO_CMD_READ_ENTITY_LIST = 5,
    XALO_CMD_GET_BONE_POSITIONS = 6,
    XALO_CMD_PROTECT_MEMORY = 7,
    XALO_CMD_HIDE_MEMORY = 8,
    XALO_CMD_UNHIDE_MEMORY = 9,
    XALO_CMD_CHECK_VAC = 10,
    XALO_CMD_GET_VIEW_MATRIX = 11,
    XALO_CMD_GET_LOCAL_PLAYER = 12,
    XALO_CMD_READ_BULK = 13,
    XALO_CMD_WRITE_BULK = 14,
    XALO_CMD_ENCRYPT_DRIVER = 15,
    XALO_CMD_DECRYPT_DRIVER = 16,
    XALO_CMD_HIDE_DRIVER = 17,
    XALO_CMD_SHOW_DRIVER = 18,
    XALO_CMD_SHUTDOWN = 19
} XALO_COMMAND;

// ==================== İLETİŞİM PAKETİ ====================

typedef struct _XALO_COMMAND_PACKET {
    UINT64 Command;
    UINT64 ProcessId;
    UINT64 SourceAddress;
    UINT64 TargetAddress;
    UINT64 Size;
    UINT64 Status;
    UINT64 Data1;
    UINT64 Data2;
    UINT64 XorChecksum;
} XALO_COMMAND_PACKET, *PXALO_COMMAND_PACKET;

// ==================== ENTITY BİLGİSİ ====================

typedef struct _XALO_ENTITY_INFO {
    UINT64 PawnAddress;
    UINT64 ControllerAddress;
    UINT32 Health;
    UINT32 Team;
    UINT32 Flags;
    UINT32 IsAlive;
    UINT32 IsSpotted;
    UINT32 Padding1;
    FLOAT ViewOffsetZ;
    FLOAT Position[3];
    FLOAT Padding2;
} XALO_ENTITY_INFO, *PXALO_ENTITY_INFO;

// ==================== KEMİK POZİSYONU ====================

typedef struct _XALO_BONE_POSITION {
    FLOAT Position[3];
    FLOAT Padding;
    UINT32 Valid;
    UINT32 Padding2[3];
} XALO_BONE_POSITION, *PXALO_BONE_POSITION;

// ==================== OYUNCU BİLGİSİ ====================

typedef struct _XALO_PLAYER_INFO {
    UINT64 PawnAddress;
    UINT64 ControllerAddress;
    UINT32 Health;
    UINT32 Team;
    UINT32 IsAlive;
    UINT32 IsSpotted;
    UINT32 IsVisible;
    FLOAT Position[3];
    FLOAT ViewOffsetZ;
    FLOAT BonePositions[XALO_MAX_BONES][3];
    FLOAT ScreenPosition[2];
    FLOAT ScreenHeadPosition[2];
    FLOAT ScreenFootPosition[2];
    FLOAT Distance;
    CHAR PlayerName[XALO_MAX_PLAYER_NAME];
    UINT32 NameLength;
    UINT32 Padding;
} XALO_PLAYER_INFO, *PXALO_PLAYER_INFO;

// ==================== MODÜL BİLGİSİ ====================

typedef struct _XALO_MODULE_INFO {
    WCHAR Name[256];
    UINT64 BaseAddress;
    UINT64 Size;
} XALO_MODULE_INFO, *PXALO_MODULE_INFO;

// ==================== OYUN DURUMU ====================

typedef struct _XALO_GAME_STATE {
    UINT64 DriverLoaded;
    UINT64 DriverXorKey;
    UINT64 Cs2ProcessId;
    UINT64 ClientBaseAddress;
    UINT64 EngineBaseAddress;
    UINT64 MatchmakingBaseAddress;
    UINT64 ClientSize;
    UINT64 EngineSize;
    UINT64 MatchmakingSize;
    UINT64 OffsetEntityList;
    UINT64 OffsetLocalPlayerPawn;
    UINT64 OffsetViewMatrix;
    UINT64 OffsetGameSceneNode;
    UINT64 OffsetModelState;
    UINT64 OffsetBoneArray;
    UINT64 LocalPlayerPawn;
    UINT64 LocalPlayerController;
    UINT32 LocalPlayerTeam;
    UINT32 LocalPlayerHealth;
    FLOAT ViewMatrix[4][4];
    UINT32 EntityCount;
    UINT32 PlayerCount;
    XALO_ENTITY_INFO Entities[XALO_MAX_ENTITIES];
    XALO_PLAYER_INFO Players[XALO_MAX_PLAYERS];
    UINT8 VisibilityMap[XALO_MAX_ENTITIES];
    UINT64 VacScanning;
    UINT64 VacScanTimestamp;
    UINT64 TotalReads;
    UINT64 TotalWrites;
    UINT64 LastReadTimestamp;
    UINT64 LastWriteTimestamp;
    UINT64 WriteLock;
    UINT64 ReadLock;
    UINT64 SequenceNumber;
    UINT8 Reserved[0x500];
} XALO_GAME_STATE, *PXALO_GAME_STATE;

// ==================== RING BUFFER ====================

typedef struct _XALO_RING_BUFFER_ENTRY {
    XALO_COMMAND_PACKET Packet;
    UINT64 SequenceNumber;
    UINT64 Timestamp;
    UINT8 Data[512];
} XALO_RING_BUFFER_ENTRY;

typedef struct _XALO_RING_BUFFER {
    XALO_RING_BUFFER_ENTRY Entries[XALO_RING_BUFFER_SIZE / sizeof(XALO_RING_BUFFER_ENTRY)];
    UINT64 ReadIndex;
    UINT64 WriteIndex;
    UINT64 Count;
} XALO_RING_BUFFER, *PXALO_RING_BUFFER;

// ==================== RENK YAPISI ====================

typedef struct _XALO_COLOR {
    FLOAT R, G, B, A;
} XALO_COLOR;

// ==================== KONFİGÜRASYON ====================

typedef struct _XALO_CONFIG {
    UINT32 ESPEnabled;
    UINT32 ESPBoxEnabled;
    UINT32 ESPHealthBarEnabled;
    UINT32 ESPNameEnabled;
    UINT32 ESPDistanceEnabled;
    UINT32 ESPHeadDotEnabled;
    UINT32 ESPLineEnabled;
    UINT32 ESPSnaplineEnabled;
    UINT32 ESPVisibleCheck;
    UINT32 ESPTeamCheck;
    UINT32 ESPEnemyOnly;
    UINT32 ESPGlowEnabled;
    XALO_COLOR ESPVisibleColor;
    XALO_COLOR ESPHiddenColor;
    XALO_COLOR ESPBoxColor;
    XALO_COLOR ESPHealthBarColor;
    XALO_COLOR ESPNameColor;
    XALO_COLOR ESPLineColor;
    XALO_COLOR ESPHeadDotColor;
    FLOAT ESPBoxThickness;
    FLOAT ESPLineThickness;
    FLOAT ESPMaxDistance;
    UINT32 AimbotEnabled;
    UINT32 AimbotTargetMode;
    UINT32 AimbotBoneTarget;
    UINT32 AimbotVisibleCheck;
    UINT32 AimbotTeamCheck;
    UINT32 AimbotSmoothingEnabled;
    FLOAT AimbotSmoothingFactor;
    FLOAT AimbotFOV;
    UINT32 AimbotRecoilControl;
    FLOAT AimbotRecoilStrength;
    UINT32 AimbotLockMode;
    UINT32 AimbotKey;
    UINT32 AimbotAutoTarget;
    UINT32 TriggerbotEnabled;
    UINT32 TriggerbotKey;
    FLOAT TriggerbotMinDelay;
    FLOAT TriggerbotMaxDelay;
    UINT32 TriggerbotVisibleCheck;
    UINT32 TriggerbotTeamCheck;
    UINT32 OverlayEnabled;
    UINT32 MenuEnabled;
    UINT32 MenuKey;
    UINT32 FPSLimit;
    XALO_COLOR FOVColor;
    FLOAT FOVCircleRadius;
    UINT32 CurrentProfile;
    UINT8 Reserved[0x400];
} XALO_CONFIG, *PXALO_CONFIG;

// ==================== YARDIMCI FONKSİYON ====================

__forceinline bool XaloWorldToScreen(
    const FLOAT world[3],
    const FLOAT viewMatrix[4][4],
    FLOAT screen[2],
    INT screenWidth,
    INT screenHeight
) {
    FLOAT clipX = world[0] * viewMatrix[0][0] + world[1] * viewMatrix[0][1] + world[2] * viewMatrix[0][2] + viewMatrix[0][3];
    FLOAT clipY = world[0] * viewMatrix[1][0] + world[1] * viewMatrix[1][1] + world[2] * viewMatrix[1][2] + viewMatrix[1][3];
    FLOAT clipW = world[0] * viewMatrix[3][0] + world[1] * viewMatrix[3][1] + world[2] * viewMatrix[3][2] + viewMatrix[3][3];
    
    if (clipW < 0.001f) {
        return false;
    }
    
    FLOAT ndcX = clipX / clipW;
    FLOAT ndcY = clipY / clipW;
    
    screen[0] = (screenWidth / 2.0f) * (ndcX + 1.0f);
    screen[1] = (screenHeight / 2.0f) * (1.0f - ndcY);
    
    return (screen[0] >= 0 && screen[0] <= screenWidth && screen[1] >= 0 && screen[1] <= screenHeight);
}