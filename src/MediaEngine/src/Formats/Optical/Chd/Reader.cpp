#include "Reader.h"
#include "../../Common/Chd/Sources.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Models/Optical/Image.h"
#include "../../../Models/Optical/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ChdOptical {
namespace {
using namespace Constants;
using OpticalInternal::add;using OpticalInternal::multiply;
std::vector<std::shared_ptr<const OpticalTrack>> createTracks(Layout& layout,std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){
    std::vector<std::shared_ptr<const OpticalTrack>> tracks;
    if(layout.dvd){if(!layout.tracks.empty()||layout.unitBytes!=Data2048||layout.logicalBytes%Data2048)throw MediaError(DiagnosticCode::ChdOpticalLayout);OpticalTrackOptions value;value.sessionNumber=value.trackNumber=1;value.mode=OpticalTrackMode::DvdData2048;value.sectorCount=layout.logicalBytes/Data2048;value.storedSectorSize=value.userDataLength=Data2048;value.dataSource=std::move(source);tracks.push_back(std::make_shared<OpticalTrack>(std::move(value)));return tracks;}
    if(layout.tracks.empty()||layout.unitBytes!=CdBytes||layout.hunkBytes%CdBytes)throw MediaError(DiagnosticCode::ChdOpticalLayout);
    std::sort(layout.tracks.begin(),layout.tracks.end(),[](const auto& a,const auto& b){return a.number<b.number;});std::int64_t chdFrames=0,physicalFrames=0;int previous=0;
    for(const auto& declaration:layout.tracks){checkCancellation(token);if(declaration.number==previous)throw MediaError(DiagnosticCode::ChdOpticalLayout);previous=declaration.number;const auto dataFrames=declaration.frames-declaration.pregap,trackOffset=multiply(add(chdFrames,declaration.pregap),CdBytes);
        OpticalTrackOptions value;value.sessionNumber=1;value.trackNumber=declaration.number;value.mode=declaration.mode;value.firstSector=add(physicalFrames,declaration.pregap);value.sectorCount=dataFrames;value.storedSectorSize=CdBytes;value.userDataLength=declaration.dataSize;value.dataSource=source;value.sourceOffset=trackOffset;
        if(declaration.mode==OpticalTrackMode::Mode1Raw2352){value.userDataOffset=RawUserOffset;value.userDataLength=Data2048;}else if(declaration.mode==OpticalTrackMode::Mode2Raw2352){value.userDataOffset=RawUserOffset;value.userDataLength=Data2336;}
        if(declaration.pregap)value.indexes.emplace_back(0,-declaration.pregap);value.indexes.emplace_back(1,0);value.pregapSectors=declaration.pregap;value.postgapSectors=declaration.postgap;
        if(declaration.subchannelSize){value.subchannelSource=source;value.subchannelSourceOffset=add(trackOffset,declaration.dataSize);value.subchannelBytesPerSector=declaration.subchannelSize;value.subchannelStride=CdBytes;}
        if(declaration.storedPregap){value.storedPregapSectors=declaration.pregap;if(declaration.pregap)value.storedPregapSourceOffset=multiply(chdFrames,CdBytes);}
        tracks.push_back(std::make_shared<OpticalTrack>(std::move(value)));physicalFrames=add(physicalFrames,declaration.frames);const auto aligned=multiply(add(declaration.frames,TrackAlignment-1)/TrackAlignment,TrackAlignment);chdFrames=add(chdFrames,aligned);
    }
    if(multiply(chdFrames,CdBytes)!=layout.logicalBytes)throw MediaError(DiagnosticCode::ChdOpticalLayout);return tracks;
}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Optical},{RepresentationKind::OpticalTracks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{readLayout(context,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;auto layout=readLayout(context,token);auto source=layout.decoded?Chd::decodedSource(context.dataSource(),token):Chd::uncompressedSource(context.dataSource(),std::uint64_t(layout.logicalBytes),layout.hunkBytes,std::move(layout.storedHunks));auto tracks=createTracks(layout,std::move(source),token);std::int64_t logical=0;for(const auto& track:tracks)logical=OpticalInternal::add(logical,OpticalInternal::multiply(track->sectorCount(),track->userDataLength()));const auto count=tracks.size();OpticalImageOptions options;options.logicalLength=logical;options.tracks=std::move(tracks);
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Optical,std::make_shared<OpticalImage>(std::move(options)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VersionKey),std::to_string(layout.version)},{std::string(CompressionKey),layout.compression},{std::string(HunkSizeKey),std::to_string(layout.hunkBytes)},{std::string(UnitSizeKey),std::to_string(layout.unitBytes)},{std::string(TrackCountKey),std::to_string(count)},{std::string(ProfileKey),std::string(layout.dvd?DvdProfile:CdProfile)}});
    });
}
}
