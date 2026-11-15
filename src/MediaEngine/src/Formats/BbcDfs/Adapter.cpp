#include "Adapter.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Sources/RandomAccessSources.h"
#include "BbcDfsImages.h"
namespace RetroMediaLab::MediaEngine::BbcDfs {
const ReaderCapabilities& Reader::capabilities()const {
    static const auto result=[]{ReaderCapabilities c;for(const auto& g:geometries())c.formatIds.push_back(g.formatId);c.extensions={std::string(Constants::Ssd),std::string(Constants::Dsd),std::string(Constants::Adl),std::string(Constants::Ima),std::string(Constants::Img)};c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return result;
}
bool Reader::supportsFormatId(std::string_view id)const{return findGeometry(id).has_value();}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{MediaFileSystems::checkCancellation(token);const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;const bool needsCatalogue=sameIdentifier(context.extension(),Constants::Ima)||sameIdentifier(context.extension(),Constants::Img)||sameIdentifier(context.extension(),Constants::Adl);const auto header=needsCatalogue?context.readHeaderAsync(Constants::TrackSize+Constants::HeaderSectors*Constants::SectorSize,token).get():std::vector<std::uint8_t>{};return canRead(header,std::uint64_t(context.length()),context.extension(),requested?std::optional<std::string_view>(*requested):std::nullopt);});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(memorySource(bytes),context.extension(),token));});
}
}
