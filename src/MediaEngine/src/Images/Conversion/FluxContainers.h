#pragma once
#include "../../Formats/Hfe/Hfe.h"
#include "../../Formats/Scp/Scp.h"
namespace RetroMediaLab::MediaEngine::FluxContainerConversion {
RETROMEDIALAB_MEDIAENGINE_API void validateParity(const Hfe::Image&,const Hfe::Image&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void validateParity(const Scp::Image&,const Scp::Image&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void validateParity(const Hfe::Image&,const Scp::Image&,std::stop_token={});
}
