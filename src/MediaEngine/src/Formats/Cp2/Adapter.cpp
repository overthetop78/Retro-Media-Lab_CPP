#include "Cp2.h"
#include "Constants.h"
#include "../Raw/RawImages.h"
#include "../Raw/Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Cp2 {
const ReaderCapabilities& Reader::capabilities()const {
    static const auto caps=[]{ReaderCapabilities value;for(const auto& geometry:Raw::ibmGeometries())value.formatIds.push_back(geometry.formatId);value.extensions.emplace_back(Constants::Extension);value.signatures.push_back({Constants::Signature.begin(),Constants::Signature.end()});value.mediaKinds={MediaKind::Floppy};value.representationKinds={RepresentationKind::Sectors};return value;}();return caps;
}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Raw::Constants::IbmPrefix);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;if(context.length()<std::int64_t(Constants::MinimumFileLength))return false;
        const auto signature=context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get();return signature.size()==Constants::Signature.size()&&std::equal(signature.begin(),signature.end(),Constants::Signature.begin());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(bytes,token));});
}
}
