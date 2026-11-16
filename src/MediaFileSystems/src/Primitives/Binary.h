#pragma once
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include "BinaryConstants.h"
#include <algorithm>
#include <bit>
#include <type_traits>
#include <array>
#include <limits>
#include <QUuid>
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Binary {
using namespace BinaryConstants;
template<class T> T little(Bytes data, std::size_t offset) {
    if (offset > data.size() || sizeof(T) > data.size() - offset) throw MediaError(DiagnosticCode::ReadOutOfBounds);
    using U=std::make_unsigned_t<T>;U value=0;
    for(std::size_t i=0;i<sizeof(T);++i)value=U(value|U(U(data[offset+i])<<(i*BitsPerByte)));
    return std::bit_cast<T>(value);
}
template<class T> void writeLittle(MutableBytes data,std::size_t offset,T value) {
    if(offset>data.size() || sizeof(T)>data.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    using U=std::make_unsigned_t<T>;
    for(std::size_t i=0;i<sizeof(T);++i)data[offset+i]=std::uint8_t(U(value)>>(i*BitsPerByte));
}
inline std::uint32_t little24(Bytes data,std::size_t offset) {
    if(offset>data.size()||UInt24Bytes>data.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    std::uint32_t value=0;
    for(std::size_t i=0;i<UInt24Bytes;++i)value|=std::uint32_t(data[offset+i])<<(i*BitsPerByte);
    return value;
}

template<class T> T big(Bytes data,std::size_t offset) {
    if(offset>data.size()||sizeof(T)>data.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    using U=std::make_unsigned_t<T>;U value=0;
    for(std::size_t i=0;i<sizeof(T);++i)value=U((value<<BitsPerByte)|data[offset+i]);
    return std::bit_cast<T>(value);
}
template<class T> void writeBig(MutableBytes data,std::size_t offset,T value) {
    if(offset>data.size()||sizeof(T)>data.size()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    using U=std::make_unsigned_t<T>;
    for(std::size_t i=0;i<sizeof(T);++i)data[offset+i]=std::uint8_t(U(value)>>((sizeof(T)-1-i)*BitsPerByte));
}
inline std::uint32_t crc32(Bytes data) {
    auto crc = std::numeric_limits<std::uint32_t>::max();
    for (auto value : data) {
        crc ^= value;
        for (std::size_t bit = 0; bit < BitsPerByte; ++bit) crc = (crc >> 1) ^ ((crc & 1) ? std::uint32_t(CrcPolynomial) : 0);
    }
    return ~crc;
}
inline std::string guid(Bytes data) {
    if (data.size() < GuidLength) throw MediaError(DiagnosticCode::ReadOutOfBounds);
    auto reordered = std::array<std::uint8_t, GuidLength>{};
    std::copy_n(data.begin(), GuidLength, reordered.begin());
    std::reverse(reordered.begin(), reordered.begin() + GuidFirstWord);
    std::reverse(reordered.begin() + GuidFirstWord, reordered.begin() + GuidSecondWord);
    std::reverse(reordered.begin() + GuidSecondWord, reordered.begin() + GuidTailOffset);
    const auto value = QUuid::fromRfc4122(QByteArrayView(reinterpret_cast<const char*>(reordered.data()), qsizetype(reordered.size())));
    return value.toString(QUuid::WithoutBraces).toStdString();
}
inline bool zero(Bytes data) { return std::all_of(data.begin(), data.end(), [](auto value) { return value == 0; }); }
}
