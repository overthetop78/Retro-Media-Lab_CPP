#pragma once
#include "Export.h"
#include <cstdint>
#include <optional>
namespace RetroMediaLab::MediaEngine {
enum class RepresentationKind {Flux,Sectors,Blocks,OpticalTracks,Sequential};
class RETROMEDIALAB_MEDIAENGINE_NOVTABLE RepresentationInfo {
public:
    virtual ~RepresentationInfo()=default;
    virtual RepresentationKind representationKind()const noexcept=0;
    virtual std::optional<std::int64_t> logicalLength()const=0;
    virtual bool supportsRandomAccess()const=0;
    virtual bool supportsSequentialAccess()const=0;
};
}
