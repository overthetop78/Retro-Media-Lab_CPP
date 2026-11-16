#pragma once
#include "Decoder.h"
namespace RetroMediaLab::MediaEngine::AppleScp {
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> appleII(const Scp::Image&,const Decoder&,bool prodosOrder,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> rwts18(const Scp::Image&,const Decoder&,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> macintosh(const Scp::Image&,const Decoder&,const std::optional<std::string>& requested,const IsoReconstruction::CollectionProgressCallback& = {},std::stop_token={});
}
