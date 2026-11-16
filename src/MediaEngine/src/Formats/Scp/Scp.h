#pragma once
#include "Constants.h"
#include "../../Models/Flux/Flux.h"
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaEngine/ScpHeader.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <functional>
#include <string>
namespace RetroMediaLab::MediaEngine::Flux {class ProtectedTrack;}
namespace RetroMediaLab::MediaEngine::Scp {
enum class RevolutionOrigin {Captured,Synthetic};
struct Revolution {
    Flux::Revolution flux;
    std::uint32_t declaredFluxCount=0;
    RevolutionOrigin origin=RevolutionOrigin::Captured;
    RETROMEDIALAB_MEDIAENGINE_API double durationMilliseconds(std::int32_t resolutionNanoseconds) const;
    RETROMEDIALAB_MEDIAENGINE_API double rpm(std::int32_t resolutionNanoseconds) const;
};
struct Track {
    std::uint8_t trackNumber=0;
    std::int32_t cylinder=0,head=0;
    std::vector<Revolution> revolutions;
};
struct Image {Header header;std::vector<Track> tracks;bool checksumValid=false;std::uint64_t fileSize=0;};
using SharedImage=std::shared_ptr<const Image>;
struct DecodeWindow {Flux::Revolution flux;std::int32_t revolution=0;bool continuous=false;};
RETROMEDIALAB_MEDIAENGINE_API void visitDecodeWindows(const Flux::ProtectedTrack&,const std::function<void(const DecodeWindow&)>&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<DecodeWindow> decodeWindows(const Flux::ProtectedTrack&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API bool hasSignature(MediaFileSystems::Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool canRead(const MediaFileSystems::RandomAccessData&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Header readHeader(MediaFileSystems::Bytes);
RETROMEDIALAB_MEDIAENGINE_API Image read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Image read(const MediaFileSystems::RandomAccessData&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t updateChecksum(std::uint32_t,MediaFileSystems::Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool isChecksumValid(std::uint32_t declared,std::uint8_t flags,std::uint32_t computed) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::pair<std::int32_t,std::int32_t> trackAddress(std::int32_t);
// The window owns its buffers; the callback may copy it when a longer lifetime is needed.
RETROMEDIALAB_MEDIAENGINE_API void visitDecodeWindows(const Track&,const std::function<void(const DecodeWindow&)>&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<DecodeWindow> decodeWindows(const Track&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API DecodeWindow primaryWindow(const Track&,std::stop_token={});
}
