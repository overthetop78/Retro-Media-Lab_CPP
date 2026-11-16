#pragma once
#include "Iso/CandidateDecoder.h"
#include "../Decoding/Registry.h"
#include "../Formats/Definitions/DiskDefinitions.h"
namespace RetroMediaLab::MediaEngine::DefinedReconstruction {
class Reader final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Reader(Decoding::Registry = {},IsoReconstruction::ReadCapture={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const std::filesystem::path&,std::string_view,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Flux::ProtectedTrackImage&,std::string_view,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Flux::ProtectedTrackImage&,const Definitions::DiskDefinition&,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
private:
    IsoReconstruction::ReadCapture readCapture_;
    IsoReconstruction::CandidateDecoder candidates_;
};
}
