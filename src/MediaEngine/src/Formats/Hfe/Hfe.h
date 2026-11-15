#pragma once
#include "../../Models/Flux/ProtectedTrack.h"
#include "../../Encoding/Track.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <map>
#include <filesystem>
namespace RetroMediaLab::MediaEngine::Hfe {
struct Track {std::int32_t cylinder=0,head=0;Flux::Bits bits;std::uint32_t bitCellTicks=0;std::vector<Flux::TrackTimingSegment> timing;std::vector<Flux::TrackFeature> features;Flux::Revolution revolution;};
struct Image {std::uint8_t revision=0;std::int32_t cylinders=0,heads=0;std::uint8_t encoding=0;std::uint16_t bitRate=0;std::vector<Track> tracks;bool writeProtected=false;};
using Metadata=std::map<std::string,std::string>;
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> pack(Flux::BitView);
RETROMEDIALAB_MEDIAENGINE_API Flux::Bits unpack(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API Track uniformTrack(std::int32_t,std::int32_t,Flux::Bits,std::uint32_t);
RETROMEDIALAB_MEDIAENGINE_API Image read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Metadata metadata(const Image&);
RETROMEDIALAB_MEDIAENGINE_API Flux::ProtectedTrackImage protectedImage(const Image&);
RETROMEDIALAB_MEDIAENGINE_API Image fromProtected(const Flux::ProtectedTrackImage&,const Metadata&);
RETROMEDIALAB_MEDIAENGINE_API bool canRepresentExactly(const Flux::ProtectedTrackImage&);
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t resolveEncoding(const Metadata&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const Image&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(std::span<const Encoding::DiskTrack>);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> buildV3(const Flux::ProtectedTrackImage&,const Metadata&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const Flux::ProtectedTrackImage&,const Metadata&);
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(std::span<const Encoding::DiskTrack>,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void write(const Flux::ProtectedTrackImage&,const Metadata&,const std::filesystem::path&,std::stop_token={});
std::vector<std::uint8_t> header(const Image&);
void appendTrackData(std::vector<std::uint8_t>&,const std::vector<std::vector<std::uint8_t>>&,std::size_t,std::uint8_t);
}
