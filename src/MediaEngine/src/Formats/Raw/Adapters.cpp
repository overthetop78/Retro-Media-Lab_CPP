#include "Adapters.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Sources/RandomAccessSources.h"
#include "RawImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
namespace RetroMediaLab::MediaEngine::Raw {
namespace {
ReaderCapabilities caps(bool msx){ReaderCapabilities c;for(const auto& g:msx?msxGeometries():ibmGeometries())c.formatIds.push_back(g.formatId);c.extensions=msx?std::vector<std::string>{std::string(Constants::Dsk)}:std::vector<std::string>{std::string(Constants::Ima),std::string(Constants::Img)};c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}
}
const ReaderCapabilities& Reader::capabilities()const{static const auto ibm=caps(false),msx=caps(true);return kind_==ReaderKind::Ibm?ibm:msx;}
bool Reader::supportsFormatId(std::string_view id)const{const auto prefix=kind_==ReaderKind::Ibm?Constants::IbmPrefix:Constants::MsxPrefix;return id.size()>=prefix.size()&&Primitives::ordinalIdentifierEqualsIgnoreCase(id.substr(0,prefix.size()),prefix);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[kind=kind_,context,token]{const auto requested=context.requestedFormatId();if(requested&&!Reader{kind}.supportsFormatId(*requested))return false;
        if(kind==ReaderKind::Msx&&context.length()>std::numeric_limits<std::int32_t>::max())return false;
        const auto bytes=context.readBytesAsync(token).get();
        if(kind==ReaderKind::Msx)return looksLikeMsx(bytes,bytes.size())&&bytes.size()>Constants::DescriptorOffset&&msxFromCapacity(bytes.size(),bytes[Constants::DescriptorOffset]).has_value();
        try{detectIbm(bytes,bytes.size());return true;}catch(const MediaError& error){if(error.diagnostic().code==DiagnosticCode::RawInvalidLength||error.diagnostic().code==DiagnosticCode::RawUnknownGeometry)return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[kind=kind_,context,token]{const auto bytes=context.readBytesAsync(token).get();const auto source=memorySource(bytes);return floppySectorDocument(context.source(),kind==ReaderKind::Ibm?readIbm(source,token):readMsx(source,token));});
}
}
