#pragma once
#include <QStringDecoder>
#include <QList>
#include <string_view>
#include "IdentifierConstants.h"

namespace RetroMediaLab::MediaEngine::Primitives {
inline QString utf8Identifier(std::string_view id) {
    // An identifier is content, not a text file: retain an initial U+FEFF.
    QStringDecoder decoder(QStringConverter::Utf8, QStringConverter::Flag::ConvertInitialBom);
    return decoder(QByteArrayView(id.data(), qsizetype(id.size())));
}
inline bool isBlankIdentifier(std::string_view id) {
    return utf8Identifier(id).trimmed().isEmpty();
}
inline char foldAsciiIdentifierCharacter(char value) noexcept {
    return value >= IdentifierConstants::UpperFirst && value <= IdentifierConstants::UpperLast
        ? char(value + IdentifierConstants::CaseDelta) : value;
}
// The catalogue keys are ASCII. Do not equate non-ASCII characters with ASCII letters.
inline bool asciiIdentifierEqualsIgnoreCase(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) return false;
    for (std::size_t index = 0; index < left.size(); ++index)
        if (foldAsciiIdentifierCharacter(left[index]) != foldAsciiIdentifierCharacter(right[index])) return false;
    return true;
}
inline bool asciiIdentifierStartsWithIgnoreCase(std::string_view id, std::string_view prefix) noexcept {
    return id.size() >= prefix.size() && asciiIdentifierEqualsIgnoreCase(id.substr(0, prefix.size()), prefix);
}
inline bool ordinalIdentifierCharacterEquals(char32_t left, char32_t right) noexcept {
    if (left == right) return true;
    if (left < IdentifierConstants::AsciiLimit || right < IdentifierConstants::AsciiLimit)
        return left < IdentifierConstants::AsciiLimit && right < IdentifierConstants::AsciiLimit
            && foldAsciiIdentifierCharacter(char(left)) == foldAsciiIdentifierCharacter(char(right));
    return QChar::toUpper(left) == QChar::toUpper(right);
}
inline bool ordinalIdentifierEqualsIgnoreCase(std::string_view left, std::string_view right) {
    const auto a = utf8Identifier(left).toUcs4(), b = utf8Identifier(right).toUcs4();
    if (a.size() != b.size()) return false;
    for (qsizetype index = 0; index < a.size(); ++index)
        if (!ordinalIdentifierCharacterEquals(a[index], b[index])) return false;
    return true;
}
inline bool ordinalIdentifierStartsWithIgnoreCase(std::string_view id, std::string_view prefix) {
    const auto a = utf8Identifier(id).toUcs4(), b = utf8Identifier(prefix).toUcs4();
    if (a.size() < b.size()) return false;
    for (qsizetype index = 0; index < b.size(); ++index)
        if (!ordinalIdentifierCharacterEquals(a[index], b[index])) return false;
    return true;
}
}
