#pragma once
#include <QStringDecoder>
#include <string_view>

namespace RetroMediaLab::MediaFileSystems::Identifiers {
inline QString utf8(std::string_view value) {
    QStringDecoder decoder(QStringConverter::Utf8, QStringConverter::Flag::ConvertInitialBom);
    return decoder(QByteArrayView(value.data(), qsizetype(value.size())));
}
inline bool isBlank(std::string_view value) { return utf8(value).trimmed().isEmpty(); }
}
