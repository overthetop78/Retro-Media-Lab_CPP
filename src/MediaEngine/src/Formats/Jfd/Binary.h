#pragma once
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <span>
namespace RetroMediaLab::MediaEngine::Jfd::Binary {
inline void require(std::span<const std::uint8_t> bytes,std::size_t position,std::size_t count,std::size_t end) {
    if(end>bytes.size()||position>end||count>end-position)throw MediaError(DiagnosticCode::JfdTruncated);
}
inline std::uint32_t read(std::span<const std::uint8_t> bytes,std::size_t at) {
    require(bytes,at,Constants::FieldBytes,bytes.size());std::uint32_t value{};
    for(std::size_t index=0;index<Constants::FieldBytes;++index)value|=std::uint32_t(bytes[at+index])<<(index*Constants::NumberShift);
    return value;
}
inline void put(std::span<std::uint8_t> bytes,std::size_t at,std::uint32_t value) {
    require(bytes,at,Constants::FieldBytes,bytes.size());
    for(std::size_t index=0;index<Constants::FieldBytes;++index)bytes[at+index]=std::uint8_t(value>>(index*Constants::NumberShift));
}
inline std::size_t offset(std::span<const std::uint8_t> bytes,std::size_t at) {
    const auto value=read(bytes,at);if(value>bytes.size())throw MediaError(DiagnosticCode::JfdOffset);return value;
}
}
