#include "Parser.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::SimhTape {
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities value{{std::string(FormatId)},{std::string(Extension)},{},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return value;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        checkCancellation(token);const auto requested=context.requestedFormatId();if(requested&&!sameIdentifier(*requested,Constants::FormatId))return false;
        try {parse(context,false,token);return true;}catch(const MediaError& error){if(error.diagnostic().code==DiagnosticCode::SimhData)return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        auto result=parse(context,true,token);return std::make_shared<const MediaDocument>(context.source(),std::string(Constants::FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),std::nullopt,std::move(result.segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(result.messages));
    });
}
}
