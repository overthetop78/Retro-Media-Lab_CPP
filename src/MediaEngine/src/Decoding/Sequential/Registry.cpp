#include "Registry.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
SequentialDecoderRegistry::SequentialDecoderRegistry(std::vector<std::shared_ptr<const SequentialDecoder>> decoders):decoders_(std::move(decoders)) {
    for(const auto& decoder:decoders_)if(!decoder)throw MediaError(DiagnosticCode::SequentialDecoderNull);
}
std::vector<std::shared_ptr<const SequentialDecoder>> SequentialDecoderRegistry::findCompatible(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const {
    checkCancellation(token);std::vector<std::shared_ptr<const SequentialDecoder>> compatible;
    for(const auto& decoder:decoders_){checkCancellation(token);if(decoder->canDecode(document,machine))compatible.push_back(decoder);}return compatible;
}
std::shared_ptr<const SequentialDecoder> SequentialDecoderRegistry::getRequired(const MediaDocument& document,std::optional<std::string> machine,std::stop_token token)const {
    const auto compatible=findCompatible(document,std::move(machine),token);
    if(compatible.empty())throw MediaError(DiagnosticCode::SequentialDecoderMissing,{document.formatId()});
    if(compatible.size()>1)throw MediaError(DiagnosticCode::SequentialDecoderAmbiguous,{document.formatId()});return compatible.front();
}
std::future<SequentialDecodeResult> SequentialDecoderRegistry::decodeAsync(std::shared_ptr<const MediaDocument> document,std::optional<std::string> machine,std::stop_token token)const {
    if(!document)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::DocumentNull);
    const auto decoder=getRequired(*document,machine,token);
    return std::async(std::launch::deferred,[decoder,document=std::move(document),machine=std::move(machine),token]{checkCancellation(token);return decoder->decodeAsync(*document,machine,token).get();});
}
}
