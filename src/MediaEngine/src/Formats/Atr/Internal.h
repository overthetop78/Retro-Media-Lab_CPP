#pragma once
#include "Atr.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::Atr::Internal {
inline std::uint16_t word(MediaFileSystems::Bytes b,std::size_t i){return std::uint16_t(b[i]|(std::uint16_t(b[i+1])<<Constants::BitsPerByte));}
inline std::uint64_t declared(MediaFileSystems::Bytes b){return ((std::uint64_t(word(b,Constants::ParagraphHigh))<<Constants::HighWordShift)|word(b,Constants::ParagraphLow))*Constants::ParagraphSize;}
}
