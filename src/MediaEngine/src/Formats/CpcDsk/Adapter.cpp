#include "Adapter.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "CpcDsk.h"
namespace RetroMediaLab::MediaEngine::CpcDsk {
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities result{{std::string(Constants::FormatId)},
        {std::string(Constants::Extensions[0]),std::string(Constants::Extensions[1])},
        {{Constants::StandardSignature.begin(),Constants::StandardSignature.end()},{Constants::ExtendedSignature.begin(),Constants::ExtendedSignature.end()}},
        {},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    return result;
}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
        if(context.length()<Constants::DiskInformationBlockSize)return false;
        const auto header=context.readHeaderAsync(Constants::DiskSignatureLength,token).get();
        return canRead(header,std::uint64_t(context.length()));
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(bytes,token));
    });
}
}
