// File: xaloAC/UserMode/XorStr.h
#pragma once
// xaloAC - Derleme Zamanında String Şifreleme

#include <string>
#include <array>
#include <cstdint>

// XOR string şifreleme için compile-time sabit
constexpr char XorKey = 0x5A;

// Compile-time XOR string sınıfı
template<size_t N>
class XorString {
private:
    std::array<char, N> m_encrypted;
    
    constexpr char EncryptChar(char c, size_t index) const {
        return c ^ (XorKey + static_cast<char>(index));
    }
    
public:
    constexpr XorString(const char(&str)[N]) : m_encrypted{} {
        for (size_t i = 0; i < N; i++) {
            m_encrypted[i] = EncryptChar(str[i], i);
        }
    }
    
    std::string Decrypt() const {
        std::string result;
        result.reserve(N - 1);
        
        for (size_t i = 0; i < N - 1; i++) {
            result.push_back(m_encrypted[i] ^ (XorKey + static_cast<char>(i)));
        }
        
        return result;
    }
    
    constexpr size_t Size() const {
        return N - 1;
    }
};

// XOR string makrosu
#define XOR_STR(str) (XorString<sizeof(str)>(str).Decrypt())

// Wide string versiyonu
template<size_t N>
class XorWideString {
private:
    std::array<wchar_t, N> m_encrypted;
    
    constexpr wchar_t EncryptChar(wchar_t c, size_t index) const {
        return c ^ (XorKey + static_cast<wchar_t>(index));
    }
    
public:
    constexpr XorWideString(const wchar_t(&str)[N]) : m_encrypted{} {
        for (size_t i = 0; i < N; i++) {
            m_encrypted[i] = EncryptChar(str[i], i);
        }
    }
    
    std::wstring Decrypt() const {
        std::wstring result;
        result.reserve(N - 1);
        
        for (size_t i = 0; i < N - 1; i++) {
            result.push_back(m_encrypted[i] ^ (XorKey + static_cast<wchar_t>(i)));
        }
        
        return result;
    }
};

#define XOR_WSTR(str) (XorWideString<sizeof(str)/sizeof(wchar_t)>(str).Decrypt())