#pragma once
#include "Stream.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::HxcStream {
void appendLittleEndian(std::vector<std::uint8_t>&,std::uint32_t,std::size_t width=Constants::FieldBytes);
void appendBlock(std::vector<std::uint8_t>&,std::uint32_t,std::span<const std::uint8_t>,std::optional<std::uint32_t> count={});
void appendPulse(std::vector<std::uint8_t>&,std::uint32_t);
inline void requireBytes(std::span<const std::uint8_t> bytes,std::size_t at,std::size_t count,std::size_t end){if(end>bytes.size()||at>end||count>end-at)throw MediaError(DiagnosticCode::HxcTruncated);}
inline std::uint32_t readU32(std::span<const std::uint8_t> bytes,std::size_t at){requireBytes(bytes,at,Constants::FieldBytes,bytes.size());std::uint32_t value{};for(std::size_t i=0;i<Constants::FieldBytes;++i)value|=std::uint32_t(bytes[at+i])<<(i*Constants::ByteBits);return value;}
void decodePulses(std::span<const std::uint8_t>,std::uint32_t,std::vector<std::uint32_t>&,std::stop_token);
void readMetadata(std::span<const std::uint8_t>,Metadata&,std::int32_t&);
std::vector<Flux::TrackFluxRevolution> createRevolutions(std::span<const std::uint32_t>,std::span<const std::uint16_t>,std::int32_t,std::stop_token);
struct TrackFile {std::filesystem::path path;std::int32_t cylinder=0,head=0;};
std::vector<TrackFile> findTrackFiles(const std::filesystem::path&);
}
