#pragma once
// Technical constants; displayed text is resolved through translation catalogs.
namespace RetroMediaLab::DiagnosticConstants {
inline constexpr auto ErrorConsoleFormat = "%s\n";
inline constexpr auto WideNullCharacter = L'\0';
inline constexpr auto NullCharacter = '\0';
inline constexpr auto ContextSeparator = '\x1f';
inline constexpr auto EnUS = "en-US";
inline constexpr auto ErrorsUnexpectedDetails = "Errors.UnexpectedDetails";
inline constexpr auto ErrorsDialogTitle = "Errors.DialogTitle";
inline constexpr auto Detail = "{detail}";
inline constexpr auto Code = "{code}";
inline constexpr auto ErrorsNativeFailure = "Errors.NativeFailure";
inline constexpr auto Underscore = '_';
inline constexpr auto LanguageSeparator = '-';
}
