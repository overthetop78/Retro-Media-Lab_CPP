#include "Reader.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/BbcDfs.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AcornAtom {
namespace {
bool looksLike(MediaFileSystems::Bytes bytes){if(bytes.size()<Constants::BlockSize*Constants::HeaderBlocks)return false;const auto fields=MediaFileSystems::BbcDfs::decodeCatalogue(bytes.subspan(Constants::BlockSize));return fields.countValid&&((fields.totalSectors==0&&fields.entryCount>0)||fields.totalSectors==Constants::Cylinders*Constants::Heads*Constants::SectorsPerTrack);}
}
const ReaderCapabilities& Reader::capabilities()const{static const auto value=[]{ReaderCapabilities c;c.formatIds={std::string(Constants::FormatId)};for(auto e:Constants::Extensions)c.extensions.emplace_back(e);c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return value;}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;if(std::uint64_t(context.length())!=Constants::Capacity||!std::any_of(Constants::Extensions.begin(),Constants::Extensions.end(),[&](auto ext){return Primitives::ordinalIdentifierEqualsIgnoreCase(ext,context.extension());}))return false;const auto header=context.readAsync(0,Constants::HeaderBlocks*Constants::BlockSize,token).get();return looksLike(header);});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();if(bytes.size()!=Constants::Capacity||!looksLike(bytes))throw MediaError(DiagnosticCode::AtomDskCatalogue,{std::to_string(bytes.size())});return floppySectorDocument(context.source(),sourceSectorImage(memorySource(bytes),{std::string(Constants::FormatId),Constants::BlockSize,Constants::Cylinders,Constants::Heads,Constants::SectorsPerTrack},0,{},SourceSectorMetadata{Constants::FirstSector,{},true}));});}
}
