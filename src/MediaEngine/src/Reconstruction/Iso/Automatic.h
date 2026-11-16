#pragma once
#include "Builder.h"
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildAutomatic(const CandidateSet&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildAtari8Bit(const CandidateSet&,std::optional<std::string> format={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildSt(const CandidateSet&,std::optional<std::string> format={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildBbc(const CandidateSet&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildIbmAutomatic(const CandidateSet&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildEpson(std::string_view,const Candidates&,std::stop_token={});
}
