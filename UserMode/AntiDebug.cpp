// File: xaloAC/UserMode/AntiDebug.cpp
// xaloAC - Anti-Debug Kontrolleri Uygulaması

#include "AntiDebug.h"
#include "XorStr.h"

namespace XaloAntiDebug {
    
    BOOL IsDebuggerPresentPEB() {
        // PEB->BeingDebugged bayrağını kontrol et
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        
        if (peb && peb->BeingDebugged) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    BOOL HasDebuggerFlags() {
        // PEB->NtGlobalFlag kontrolü
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        
        if (!peb) {
            return FALSE;
        }
        
        // NtGlobalFlag debug bayrakları:
        // FLG_HEAP_ENABLE_TAIL_CHECK (0x10)
        // FLG_HEAP_ENABLE_FREE_CHECK (0x20)
        // FLG_HEAP_VALIDATE_PARAMETERS (0x40)
        ULONG ntGlobalFlag = peb->NtGlobalFlag;
        
        if (ntGlobalFlag & 0x70) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    BOOL IsRemoteDebuggerPresent() {
        BOOL debuggerPresent = FALSE;
        
        if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent)) {
            return debuggerPresent;
        }
        
        return FALSE;
    }
    
    BOOL HasTimingAnomaly() {
        // Debugger altında çalışırken zamanlama farklılıkları olur
        LARGE_INTEGER start, end, freq;
        
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        
        // Kısa bir işlem yap
        volatile int dummy = 0;
        for (int i = 0; i < 1000; i++) {
            dummy += i;
        }
        
        QueryPerformanceCounter(&end);
        
        double elapsedMs = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
        
        // Normalde bu işlem < 1ms sürer
        // Debugger altında > 10ms sürebilir
        if (elapsedMs > 10.0) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    BOOL HasHardwareBreakpoints() {
        // Debug register'larını kontrol et
        // Dr0-Dr3 hardware breakpoint adresleri içerir
        
        CONTEXT context = {0};
        context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        
        if (!GetThreadContext(GetCurrentThread(), &context)) {
            return FALSE;
        }
        
        // Herhangi bir hardware breakpoint set edilmiş mi?
        if (context.Dr0 != 0 || context.Dr1 != 0 || context.Dr2 != 0 || context.Dr3 != 0) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    BOOL IsAnyDebuggerPresent() {
        // Tüm kontrolleri yap
        if (IsDebuggerPresentPEB()) {
            return TRUE;
        }
        
        if (HasDebuggerFlags()) {
            return TRUE;
        }
        
        if (IsRemoteDebuggerPresent()) {
            return TRUE;
        }
        
        if (HasTimingAnomaly()) {
            return TRUE;
        }
        
        if (HasHardwareBreakpoints()) {
            return TRUE;
        }
        
        return FALSE;
    }
    
    void InitializeAntiDebug() {
        // Anti-debug başlat
        // Debugger tespit edilirse uygulamayı kapat
        
        if (IsAnyDebuggerPresent()) {
            // Sessizce çık - debugger'a bilgi verme
            ExitProcess(0xDEAD);
        }
        
        // PEB->BeingDebugged'ı temizle (debugger'dan kaçınmak için)
        PPEB peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
        if (peb) {
            peb->BeingDebugged = 0;
        }
        
        // NtGlobalFlag'ı temizle
        if (peb) {
            peb->NtGlobalFlag &= ~0x70;
        }
        
        // Heap flag'lerini temizle
        // ProcessHeap->Flags ve ForceFlags
        if (peb && peb->ProcessHeap) {
            PULONG heapFlags = reinterpret_cast<PULONG>(peb->ProcessHeap);
            heapFlags[0x40 / 4] &= ~0x70; // Flags
            heapFlags[0x44 / 4] &= ~0x70; // ForceFlags
        }
    }
}