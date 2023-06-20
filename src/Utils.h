#pragma once
#include "includes.h"

template <typename T, typename R>
T cast(R const v) { return reinterpret_cast<T>(v); }

std::string narrow(const wchar_t* str);
inline auto narrow(const std::wstring& str) { return narrow(str.c_str()); }
std::wstring widen(const char* str);
inline auto widen(const std::string& str) { return widen(str.c_str()); }