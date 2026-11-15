#include "Adapter.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "Hfe.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Hfe {
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities result{{std::string(Constants::FormatId)},{std::string(Constants::Extension)},
        {{Constants::Signature.begin(),Constants::Signature.end()},{Constants::Signature3.begin(),Constants::Signature3.end()}},
        {},{MediaKind::Floppy},{RepresentationKind::Flux}};
    return result;
}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
        if(context.length()<Constants::BlockSize)return false;
        const auto header=context.readHeaderAsync(Constants::SignatureLength,token).get();
        return std::equal(header.begin(),header.end(),Constants::Signature.begin(),Constants::Signature.end())||std::equal(header.begin(),header.end(),Constants::Signature3.begin(),Constants::Signature3.end());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto bytes=context.readBytesAsync(token).get();const auto image=read(bytes,token);
        return floppyFluxDocument(context.source(),std::string(Constants::FormatId),std::make_shared<const Flux::ProtectedTrackImage>(protectedImage(image)),metadata(image));
    });
}
}
