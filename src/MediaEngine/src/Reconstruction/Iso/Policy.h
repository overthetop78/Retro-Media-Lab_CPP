#pragma once
#include "Automatic.h"
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
enum class PolicyKind {Automatic,Generic,AcornAdfs,Atari8Bit,AtariSt,Amstrad,Ibm,Bbc,Epson,Ucsd};
struct Policy {PolicyKind kind;std::vector<std::string> decoderIds;};
RETROMEDIALAB_MEDIAENGINE_API Policy resolvePolicy(const std::optional<std::string>&);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> build(const Policy&,const std::optional<std::string>&,const CandidateSet&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildAdfs(std::string_view,const CandidateSet&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildGeneric(std::string,const CandidateSet&,bool amstrad,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildUcsd(std::string,const CandidateSet&,std::stop_token={});
}
