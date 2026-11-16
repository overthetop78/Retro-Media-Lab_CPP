#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::Adaptec {
std::uint32_t errorCheck(std::span<const std::uint8_t> bytes){
    using namespace Constants;static const auto table=[]{std::array<std::uint32_t,1<<BitsPerByte> values{};for(std::size_t i=0;i<values.size();++i){auto value=std::uint32_t(i)<<CheckHighByteShift;for(auto bit=0;bit<BitsPerByte;++bit)value=(value<<1)^((value&CheckHighBit)?CheckPolynomial:0);values[i]=value;}return values;}();
    std::uint32_t value{};for(auto byte:bytes)value=(value<<BitsPerByte)^table[std::uint8_t((value>>CheckHighByteShift)^byte)];return value;
}
void writeErrorCheck(std::span<std::uint8_t> record){if(record.size()<Constants::CheckLength)throw MediaError(DiagnosticCode::AdaptecField);qToBigEndian(errorCheck(record.first(record.size()-Constants::CheckLength)),record.data()+record.size()-Constants::CheckLength);}
}
