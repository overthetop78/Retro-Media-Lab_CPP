#pragma once
#include "Reader.h"
#include "../../Formats/Scp/CaptureReader.h"
#include "../../Decoding/Registry.h"
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
// Shares the source capture cache. Each registered callback owns its reconstructors.
class Composition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Composition(std::shared_ptr<const Scp::CaptureReader>,MediaFileSystems::SectorRegistry={},Decoding::Registry={});
    const Candidates& candidates()const noexcept{return candidates_;}
    const Reader& reader()const noexcept{return reader_;}
private:
    Candidates candidates_;
    Reader reader_;
};
}
