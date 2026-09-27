// File: xaloAC/Kernel/Communication.h
#pragma once
// xaloAC - İletişim Katmanı Başlık Dosyası

#include <ntddk.h>

namespace Communication {
    // Shared memory başlat
    NTSTATUS InitializeSharedMemory(PVOID* OutSharedMemory, SIZE_T Size);
    
    // Shared memory temizle
    void CleanupSharedMemory(PVOID SharedMemory, SIZE_T Size);
    
    // Ring buffer üzerinden komut işle
    NTSTATUS ProcessCommands(PVOID SharedMemory);
    
    // XOR şifreleme
    void XorEncryptData(PVOID Data, SIZE_T Size, UINT64 Key);
    void XorDecryptData(PVOID Data, SIZE_T Size, UINT64 Key);
    
    // Ring buffer'dan komut oku
    BOOLEAN ReadCommand(PVOID SharedMemory, PVOID CommandPacket);
    
    // Ring buffer'a yanıt yaz
    void WriteResponse(PVOID SharedMemory, PVOID ResponsePacket);
    
    // Komut işleyicileri
    NTSTATUS HandleReadMemory(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleWriteMemory(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleGetModuleBase(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleGetProcessId(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleReadEntityList(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleGetBonePositions(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleProtectMemory(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleHideMemory(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleUnhideMemory(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleCheckVac(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleGetViewMatrix(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleGetLocalPlayer(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleReadBulk(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleEncryptDriver(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleDecryptDriver(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleHideDriver(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleShowDriver(PVOID SharedMemory, PVOID Packet);
    NTSTATUS HandleShutdown(PVOID SharedMemory, PVOID Packet);
}