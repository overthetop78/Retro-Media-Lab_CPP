#pragma once
#include "DiagnosticConstants.h"
#include "Errors.h"
#include <cstdio>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace RetroMediaLab::Errors {
inline constexpr auto ConsoleFormat = DiagnosticConstants::ErrorConsoleFormat;
#if defined(_WIN32)
inline std::wstring wide(std::string_view value) {
    const auto count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(count, DiagnosticConstants::WideNullCharacter);
    if (count) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), count);
    return result;
}
inline std::string systemLanguage() {
    wchar_t locale[LOCALE_NAME_MAX_LENGTH] = {};
    if (!GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH)) return DefaultLanguage;
    const auto count = WideCharToMultiByte(CP_UTF8, 0, locale, -1, nullptr, 0, nullptr, nullptr);
    std::string language(count, DiagnosticConstants::NullCharacter);
    WideCharToMultiByte(CP_UTF8, 0, locale, -1, language.data(), count, nullptr, nullptr);
    if (!language.empty()) language.pop_back();
    return resolveLanguage(language);
}
#endif
inline void show(std::string_view language, const std::string& message) {
#if defined(_WIN32)
    const auto caption = wide(text(language, TitleKey));
    const auto content = wide(message);
    MessageBoxW(nullptr, content.c_str(), caption.c_str(), MB_OK | MB_ICONERROR);
#else
    (void)language;
#endif
    std::fprintf(stderr, ConsoleFormat, message.c_str());
}
}
