#include "SpecializedReaders.h"
#include "Constants.h"
#include "../Commodore900/CoherentImages.h"
#include "../Commodore900/Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Raw {
namespace {
MediaFileSystems::SectorGeometry ucsd(){return {std::string(MediaFileSystems::Formats::UcsdIbmMfm),Constants::SectorSize,Constants::UcsdCylinders,Constants::UcsdHeads,Constants::UcsdSectors};}
std::uint64_t capacity(const MediaFileSystems::SectorGeometry& g){return std::uint64_t(g.blockSize)*g.cylinders*g.heads*g.sectorsPerTrack;}
ReaderCapabilities caps(bool coherent){return {{std::string(coherent?MediaFileSystems::Formats::Commodore900Coherent:MediaFileSystems::Formats::UcsdIbmMfm)},coherent?std::vector<std::string>{std::string(Constants::Bin),std::string(Constants::Img)}:std::vector<std::string>{std::string(Constants::Img)},{},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};}
}
const ReaderCapabilities& SpecializedReader::capabilities()const{static const auto coherent=caps(true),ucsd=caps(false);return kind_==SpecializedKind::Coherent?coherent:ucsd;}
bool SpecializedReader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,capabilities().formatIds.front());}
std::future<bool> SpecializedReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{
    MediaFileSystems::checkCancellation(token);const auto requested=context.requestedFormatId();if(requested&&!SpecializedReader(kind).supportsFormatId(*requested))return false;
    if(kind==SpecializedKind::Ucsd)return std::uint64_t(context.length())==capacity(ucsd());
    if(std::uint64_t(context.length())>Commodore900::Constants::Capacity||context.length()%Commodore900::Constants::SectorSize)return false;
    const auto bytes=context.readBytesAsync(token).get();return Commodore900::canRead(memorySource(bytes),requested?std::optional<std::string_view>(*requested):std::nullopt,token);
});}
std::future<std::shared_ptr<const MediaDocument>> SpecializedReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{
    const auto bytes=context.readBytesAsync(token).get();const auto source=memorySource(bytes);
    if(kind==SpecializedKind::Coherent)return floppySectorDocument(context.source(),Commodore900::read(source,token));
    const auto geometry=ucsd();if(bytes.size()!=capacity(geometry))throw MediaError(DiagnosticCode::RawInvalidLength,{std::to_string(bytes.size()),std::to_string(capacity(geometry))});
    return floppySectorDocument(context.source(),sourceSectorImage(source,geometry,0,{},SourceSectorMetadata{1,{},true}));
});}
}
