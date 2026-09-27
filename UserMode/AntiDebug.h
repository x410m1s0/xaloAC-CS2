// File: xaloAC/UserMode/AntiDebug.h
#pragma once
// xaloAC - Anti-Debug Kontrolleri

#include <windows.h>
#include <winternl.h>

namespace XaloAntiDebug {
    // PEB->BeingDebugged kontrolü
    BOOL IsDebuggerPresentPEB();
    
    // NtGlobalFlag kontrolü
    BOOL HasDebuggerFlags();
    
    // CheckRemoteDebuggerPresent
    BOOL IsRemoteDebuggerPresent();
    
    // Timing check (debugger yavaşlatır)
    BOOL HasTimingAnomaly();
    
    // Hardware breakpoint kontrolü
    BOOL HasHardwareBreakpoints();
    
    // Tüm kontrolleri yap
    BOOL IsAnyDebuggerPresent();
    
    // Anti-debug başlat
    void InitializeAntiDebug();
}