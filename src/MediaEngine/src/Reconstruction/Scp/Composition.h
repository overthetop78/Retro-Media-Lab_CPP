#pragma once
#include "Reader.h"
#include "../../Formats/Scp/CaptureReader.h"
#include "../../Decoding/Registry.h"
namespace RetroMediaLab::MediaEngine::IsoReconstruction {class Reader;}
namespace RetroMediaLab::MediaEngine::AppleScp {class Reader;}
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
namespace Details {struct Services;}
// Shares the source capture cache. Each registered callback owns its reconstructors.
class Composition final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Composition(std::shared_ptr<const Scp::CaptureReader>,MediaFileSystems::SectorRegistry={},Decoding::Registry={});
    RETROMEDIALAB_MEDIAENGINE_API IsoReconstruction::Reader& isoReader()const;
    RETROMEDIALAB_MEDIAENGINE_API AppleScp::Reader& appleReader()const;
    RETROMEDIALAB_MEDIAENGINE_API Image readAmiga(const std::filesystem::path&,const Progress& = {},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API Image readCommodore(const std::filesystem::path&,std::optional<std::string> format={},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API Image readRx02(const std::filesystem::path&,std::stop_token={})const;
    const Candidates& candidates()const noexcept{return candidates_;}
    const Reader& reader()const noexcept{return reader_;}
private:
    std::shared_ptr<Details::Services> services_;
    Candidates candidates_;
    Reader reader_;
};
}
