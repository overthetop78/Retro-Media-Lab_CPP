#include "Compatibility.h"
#include "Constants.h"
#include "../../Formats/Tape/Common/Pcm.h"
#include "../../Images/RecognitionContext.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
bool canEncodeValidatedBytes(const SequentialEncodeRequest& request,std::span<const std::string> formats,std::span<const std::string> machines) {
    const auto accepts=[](std::span<const std::string> ids,std::string_view id){return std::any_of(ids.begin(),ids.end(),[&](const auto& candidate){return sameIdentifier(candidate,id);});};int rate=0;
    return accepts(formats,request.targetFormatId())&&accepts(machines,request.machineId())&&!request.blocks().empty()&&std::all_of(request.blocks().begin(),request.blocks().end(),[](const auto& block){return block.integrityValid()==true&&!block.data().empty();})&&request.retainedSegments().empty()&&(!sameIdentifier(request.targetFormatId(),SequentialEncodingConstants::WaveFormat)||TapePcm::trySampleRate(request.parameters(),rate));
}
}
