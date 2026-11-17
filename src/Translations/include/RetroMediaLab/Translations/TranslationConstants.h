#pragma once
// Technical constants; displayed text is resolved through translation catalogs.
namespace RetroMediaLab::TranslationConstants {
inline constexpr auto MediaEngine = "MediaEngine";
inline constexpr auto ArgumentPattern = "{%1}";
inline constexpr auto App = "App";
inline constexpr auto TranslationsLanguagesJson = ":/Translations/Languages.json";
inline constexpr auto Code = "code";
inline constexpr auto Culture = "culture";
inline constexpr auto NativeName = "nativeName";
inline constexpr auto EnUS = "en-US";
inline constexpr auto Underscore = '_';
inline constexpr auto LanguageSeparator = '-';
inline constexpr auto RetroMediaLabCatalog = "RetroMediaLabCatalog";
inline constexpr auto Catalogs = "catalogs";
inline constexpr auto Strings = "strings";
inline constexpr auto Invariants = "invariants";
inline constexpr auto SafeSegmentPattern = "^[A-Za-z0-9_-]+$";
inline constexpr auto Constants = "constants";
inline constexpr auto MissingKeyOpen = "[";
inline constexpr auto MissingKeyClose = "]";
namespace TranslationServiceValues {
inline constexpr auto MaximumCatalogMegabytes = 16;
inline constexpr auto BytesPerKibibyte = 1024;
}
}
