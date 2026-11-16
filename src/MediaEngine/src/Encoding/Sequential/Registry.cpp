#include "Registry.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
SequentialEncoderRegistry::SequentialEncoderRegistry(std::vector<std::shared_ptr<const SequentialEncoder>> encoders):encoders_(std::move(encoders)){for(const auto& encoder:encoders_)if(!encoder)throw MediaError(DiagnosticCode::SequentialEncoderNull);}
bool SequentialEncoderRegistry::signalTarget(const SequentialEncodeRequest& request){return request.targetFormatId()==SequentialEncodingConstants::CswFormat||request.targetFormatId()==SequentialEncodingConstants::RawFormat;}
std::vector<std::shared_ptr<const SequentialEncoder>> SequentialEncoderRegistry::findCompatible(const SequentialEncodeRequest& request,std::stop_token token)const {
    checkCancellation(token);std::optional<SequentialEncodeRequest> signal;if(signalTarget(request)){signal=request;signal->target_=SequentialEncodingConstants::WaveFormat;}const auto& selected=signal?*signal:request;
    std::vector<std::shared_ptr<const SequentialEncoder>> result;for(const auto& encoder:encoders_){checkCancellation(token);if(encoder->canEncode(selected))result.push_back(encoder);}return result;
}
std::shared_ptr<const SequentialEncoder> SequentialEncoderRegistry::getRequired(const SequentialEncodeRequest& request,std::stop_token token)const {
    const auto compatible=findCompatible(request,token);if(compatible.empty())throw MediaError(DiagnosticCode::SequentialEncoderMissing,{request.targetFormatId(),request.machineId()});if(compatible.size()>1)throw MediaError(DiagnosticCode::SequentialEncoderAmbiguous,{request.targetFormatId(),request.machineId()});return compatible.front();
}
std::future<std::shared_ptr<const SequentialImage>> SequentialEncoderRegistry::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const {
    if(signalTarget(request))request.target_=SequentialEncodingConstants::WaveFormat;const auto encoder=getRequired(request,token);
    return std::async(std::launch::deferred,[encoder,request=std::move(request),token]()mutable{checkCancellation(token);return encoder->encodeAsync(std::move(request),token).get();});
}
}
