#pragma once
#include "DiagnosticConstants.h"
#include <ErrorCatalog.h>
#include <algorithm>
#include <string>
#include <string_view>
namespace RetroMediaLab::Errors {
inline constexpr char ContextSeparator = DiagnosticConstants::ContextSeparator;
inline constexpr auto DefaultLanguage = DiagnosticConstants::EnUS;
inline constexpr auto UnexpectedKey = DiagnosticConstants::ErrorsUnexpectedDetails;
inline constexpr auto TitleKey = DiagnosticConstants::ErrorsDialogTitle;
inline constexpr auto DetailParameter = DiagnosticConstants::Detail;
inline constexpr auto CodeParameter = DiagnosticConstants::Code;
inline constexpr auto NativeFailureKey = DiagnosticConstants::ErrorsNativeFailure;
inline std::string presentationLanguage = DefaultLanguage;
inline std::string resolveLanguage(std::string language) {
    std::replace(language.begin(), language.end(), DiagnosticConstants::Underscore, DiagnosticConstants::LanguageSeparator);
    for (const auto& entry : ErrorCatalog::Entries) if (entry.language == language) return language;
    const auto prefix = language.substr(0, language.find(DiagnosticConstants::LanguageSeparator));
    for (const auto& entry : ErrorCatalog::Entries)
        if (entry.language.substr(0, entry.language.find(DiagnosticConstants::LanguageSeparator)) == prefix) return std::string(entry.language);
    return DefaultLanguage;
}
inline std::string text(std::string_view language, std::string_view key) {
    for (const auto& entry : ErrorCatalog::Entries)
        if (entry.language == language && entry.key == key) return std::string(entry.text);
    for (const auto& entry : ErrorCatalog::Entries)
        if (entry.language == DefaultLanguage && entry.key == key) return std::string(entry.text);
    return {};
}
inline void replace(std::string& value, std::string_view parameter, std::string_view content) {
    std::size_t position = 0;
    while ((position = value.find(parameter, position)) != std::string::npos) {
        value.replace(position, parameter.size(), content);
        position += content.size();
    }
}
inline std::string render(std::string_view language, std::string_view encoded) {
    const auto separator = encoded.find(ContextSeparator);
    auto result = text(language, encoded.substr(0, separator));
    const bool known = !result.empty();
    if (!known) result = text(language, UnexpectedKey);
    replace(result, DetailParameter, known ? (separator == std::string_view::npos ? std::string_view{} : encoded.substr(separator + 1)) : encoded);
    return result;
}
inline std::string nativeFailure(std::string_view language, unsigned long code, std::string_view detail) {
    auto result = text(language, NativeFailureKey);
    replace(result, DetailParameter, detail);
    replace(result, CodeParameter, std::to_string(code));
    return result;
}
}
