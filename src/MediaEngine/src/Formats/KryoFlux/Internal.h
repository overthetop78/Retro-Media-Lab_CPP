#pragma once
#include "Stream.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::KryoFlux {
inline void require(std::span<const std::uint8_t> data,std::size_t offset,std::size_t count){if(offset>data.size()||count>data.size()-offset)throw MediaError(DiagnosticCode::KryoTruncated);}
inline std::uint16_t u16(std::span<const std::uint8_t> data,std::size_t offset){require(data,offset,Constants::ShortBytes);return std::uint16_t(data[offset]|std::uint16_t(data[offset+1])<<Constants::ByteBits);}
inline std::uint32_t u32(std::span<const std::uint8_t> data,std::size_t offset){require(data,offset,Constants::IndexBytes);std::uint32_t value{};for(std::size_t i=0;i<Constants::IndexBytes;++i)value|=std::uint32_t(data[offset+i])<<(i*Constants::ByteBits);return value;}
inline std::size_t opcodeBytes(std::uint8_t code){return code==Constants::Nop3||code==Constants::Flux3?Constants::LongBytes:code<=Constants::MaximumShortOpcode||code==Constants::Nop2?Constants::ShortBytes:1;}
bool trailingFill(std::span<const std::uint8_t>,std::size_t,std::stop_token);
void readInfo(std::span<const std::uint8_t>,Metadata&,double&);
void readOutOfBand(std::span<const std::uint8_t>,std::vector<std::uint32_t>&,Metadata&,double&,std::stop_token);
struct TrackFile {std::filesystem::path path;std::int32_t cylinder,head;};
std::vector<TrackFile> findTrackFiles(const std::filesystem::path&,std::stop_token);
}
