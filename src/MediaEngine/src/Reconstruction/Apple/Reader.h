#pragma once
#include "Reconstructors.h"
#include "../Iso/CandidateDecoder.h"
#include "../../Decoding/Registry.h"
namespace RetroMediaLab::MediaEngine::AppleScp {
class Reader final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Reader(Decoding::Registry = {},IsoReconstruction::ReadCapture={},MacDecode={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const std::filesystem::path&,std::optional<std::string> format={},const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Scp::Image&,std::optional<std::string> format={},const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={})const;
private:
    Decoder decoder_;IsoReconstruction::ReadCapture readCapture_;
};
}
