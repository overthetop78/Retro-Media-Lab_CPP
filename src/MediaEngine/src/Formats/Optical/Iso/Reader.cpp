#include "Reader.h"
#include "Constants.h"
#include "../../FormatIds.h"
#include "../../../Models/Optical/Image.h"
#include <QFileInfo>
#include <QString>
namespace RetroMediaLab::MediaEngine::OpticalIso {
namespace M=MediaFileSystems;
using namespace OpticalIsoConstants;
namespace {bool lengthCompatible(std::int64_t length){return length>0&&length%SectorSize==0;}}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities value{{std::string(Formats::OpticalIso)},{std::string(IsoExtension),std::string(GcmExtension)},{},{},{MediaKind::Optical},{RepresentationKind::OpticalTracks}};return value;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{M::checkCancellation(token);if(context.requestedFormatId()&&!sameIdentifier(*context.requestedFormatId(),Formats::OpticalIso))return false;return (sameIdentifier(context.extension(),IsoExtension)||sameIdentifier(context.extension(),GcmExtension))&&lengthCompatible(context.length());});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    M::checkCancellation(token);if(!lengthCompatible(context.length()))throw M::MediaError(M::DiagnosticCode::IsoContainerLength);
    const auto source=context.dataSource();if(source->length()!=std::uint64_t(context.length()))throw M::MediaError(M::DiagnosticCode::IsoContainerChanged);
    OpticalTrackOptions track;track.sessionNumber=track.trackNumber=1;track.mode=OpticalTrackMode::Mode1Data2048;track.sectorCount=context.length()/SectorSize;track.storedSectorSize=track.userDataLength=SectorSize;track.dataSource=source;track.indexes.emplace_back(1,0);
    OpticalImageOptions optical;optical.logicalLength=context.length();optical.tracks=std::vector<std::shared_ptr<const OpticalTrack>>{std::make_shared<OpticalTrack>(std::move(track))};optical.associatedFiles=std::vector<std::string>{context.source()->primaryPath()};
    const auto suffix=QFileInfo(QString::fromUtf8(context.source()->primaryPath())).suffix();const auto extension=suffix.isEmpty()?std::string{}:std::string(1,ImageConstants::ExtensionSeparator)+suffix.toUtf8().toStdString();
    return std::make_shared<MediaDocument>(context.source(),std::string(Formats::OpticalIso),MediaKind::Optical,std::make_shared<OpticalImage>(std::move(optical)),std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>{},std::map<std::string,std::string>{{std::string(SectorSizeKey),std::to_string(SectorSize)},{std::string(TrackCountKey),std::to_string(1)},{std::string(SourceExtensionKey),extension}});
});}
}
