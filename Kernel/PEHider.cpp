// File: xaloAC/Kernel/PEHider.cpp
// xaloAC - PE Başlık Gizleyici Uygulaması

#include "PEHider.h"

// Kaydedilmiş PE başlıkları (geri yükleme için)
static IMAGE_DOS_HEADER g_SavedDosHeader;
static IMAGE_NT_HEADERS g_SavedNtHeaders;
static BOOLEAN g_HeadersSaved = FALSE;

namespace PEHider {
    
    void ErasePEHeaders(PDRIVER_OBJECT DriverObject) {
        if (!DriverObject || !DriverObject->DriverStart) {
            return;
        }
        
        PUCHAR driverBase = reinterpret_cast<PUCHAR>(DriverObject->DriverStart);
        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
        
        // Geçerli PE başlığı mı?
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return;
        }
        
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            driverBase + dosHeader->e_lfanew
        );
        
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            return;
        }
        
        // Başlıkları kaydet (geri yükleme için)
        g_SavedDosHeader = *dosHeader;
        g_SavedNtHeaders = *ntHeaders;
        g_HeadersSaved = TRUE;
        
        // PE başlıklarını sıfırla
        RtlSecureZeroMemory(driverBase, PAGE_SIZE); // İlk 4KB'ı sıfırla
        
        // e_magic'i boz
        dosHeader->e_magic = 0;
        
        // NT headers'ı sıfırla
        RtlSecureZeroMemory(ntHeaders, sizeof(IMAGE_NT_HEADERS));
    }
    
    void RestorePEHeaders(PDRIVER_OBJECT DriverObject) {
        if (!g_HeadersSaved || !DriverObject || !DriverObject->DriverStart) {
            return;
        }
        
        PUCHAR driverBase = reinterpret_cast<PUCHAR>(DriverObject->DriverStart);
        
        // Başlıkları geri yükle
        RtlCopyMemory(driverBase, &g_SavedDosHeader, sizeof(IMAGE_DOS_HEADER));
        
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            driverBase + g_SavedDosHeader.e_lfanew
        );
        
        RtlCopyMemory(ntHeaders, &g_SavedNtHeaders, sizeof(IMAGE_NT_HEADERS));
        
        g_HeadersSaved = FALSE;
    }
    
    void HideDriverSignature(PDRIVER_OBJECT DriverObject) {
        if (!DriverObject || !DriverObject->DriverStart) {
            return;
        }
        
        PUCHAR driverBase = reinterpret_cast<PUCHAR>(DriverObject->DriverStart);
        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
        
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return;
        }
        
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            driverBase + dosHeader->e_lfanew
        );
        
        // İmza bölümünü bul ve sıfırla
        // Sertifika tablosu genellikle dosyanın sonunda bulunur
        ULONG certificateTableRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress;
        ULONG certificateTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
        
        if (certificateTableRva && certificateTableSize) {
            RtlSecureZeroMemory(driverBase + certificateTableRva, certificateTableSize);
        }
    }
    
    void HideImportTable(PDRIVER_OBJECT DriverObject) {
        if (!DriverObject || !DriverObject->DriverStart) {
            return;
        }
        
        PUCHAR driverBase = reinterpret_cast<PUCHAR>(DriverObject->DriverStart);
        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
        
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return;
        }
        
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            driverBase + dosHeader->e_lfanew
        );
        
        // Import tablosunu sıfırla
        ULONG importTableRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        ULONG importTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        
        if (importTableRva && importTableSize) {
            RtlSecureZeroMemory(driverBase + importTableRva, importTableSize);
            
            // DataDirectory girişini sıfırla
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;
        }
    }
    
    void HideExportTable(PDRIVER_OBJECT DriverObject) {
        if (!DriverObject || !DriverObject->DriverStart) {
            return;
        }
        
        PUCHAR driverBase = reinterpret_cast<PUCHAR>(DriverObject->DriverStart);
        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(driverBase);
        
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return;
        }
        
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            driverBase + dosHeader->e_lfanew
        );
        
        // Export tablosunu sıfırla
        ULONG exportTableRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        ULONG exportTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        
        if (exportTableRva && exportTableSize) {
            RtlSecureZeroMemory(driverBase + exportTableRva, exportTableSize);
            
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = 0;
            ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = 0;
        }
    }
}