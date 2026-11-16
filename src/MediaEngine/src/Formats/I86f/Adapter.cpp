#include "I86f.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::I86f {
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Flux}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Primitives::ordinalIdentifierEqualsIgnoreCase(*context.requestedFormatId(),Constants::FormatId))return false;if(context.length()<std::int64_t(Constants::MinimumFileLength))return false;
        const auto signature=context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get();return signature.size()==Constants::Signature.size()&&std::equal(signature.begin(),signature.end(),Constants::Signature.begin());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();const auto image=read(bytes,token);
        return floppyFluxDocument(context.source(),std::string(Constants::FormatId),std::make_shared<const Flux::ProtectedTrackImage>(protectedImage(image,token)),{{std::string(Constants::FlagsKey),std::to_string(image.flags)}});
    });
}
}
