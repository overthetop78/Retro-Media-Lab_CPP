#pragma once
#include "../Iso/Collector.h"
#include "../../Formats/Scp/Scp.h"
namespace RetroMediaLab::MediaEngine::AppleScp {
using MacDecode=std::function<Decoding::Result(const Flux::Revolution&,double,std::stop_token)>;
class Decoder final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Decoder(Decoding::DecodeOperation,MacDecode={});
    RETROMEDIALAB_MEDIAENGINE_API IsoReconstruction::Candidates collect(const Scp::Image&,std::string_view id,std::int32_t size,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decodeMacTrack(const Scp::Track&,const Flux::Revolution&,std::stop_token={})const;
private:
    Decoding::DecodeOperation decode_;MacDecode mac_;
};
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::SectorBlock select(std::uint64_t logical,MediaFileSystems::SectorAddress,std::span<const IsoReconstruction::Candidate>,std::stop_token={});
}
