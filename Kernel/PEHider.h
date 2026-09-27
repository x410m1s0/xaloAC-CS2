// File: xaloAC/Kernel/PEHider.h
#pragma once
// xaloAC - PE Başlık Gizleyici Başlık Dosyası

#include <ntddk.h>

namespace PEHider {
    // PE başlıklarını bellekten sil
    void ErasePEHeaders(PDRIVER_OBJECT DriverObject);
    
    // PE başlıklarını geri yükle
    void RestorePEHeaders(PDRIVER_OBJECT DriverObject);
    
    // Sürücü imzasını gizle
    void HideDriverSignature(PDRIVER_OBJECT DriverObject);
    
    // Import tablosunu gizle
    void HideImportTable(PDRIVER_OBJECT DriverObject);
    
    // Export tablosunu gizle
    void HideExportTable(PDRIVER_OBJECT DriverObject);
}