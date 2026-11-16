#include "Container.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Primitives/Identifiers.h"
#include "../../../Models/Optical/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Gdi {
namespace {
std::string utf8(const std::filesystem::path& path){const auto s=path.u8string();return {s.begin(),s.end()};}
std::filesystem::path pathOf(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{std::string(BinExtension),std::string(RawExtension)},{MediaKind::Optical},{RepresentationKind::OpticalTracks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;if(!sameIdentifier(context.extension(),Constants::Extension))return false;
        try{const auto path=pathOf(context.source()->primaryPath());const auto descriptor=readDescriptor(path,token);associatedPaths(path,descriptor);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto path=pathOf(context.source()->primaryPath());const auto descriptor=readDescriptor(path,token);const auto paths=associatedPaths(path,descriptor);
        std::vector<std::pair<std::string,std::shared_ptr<const MediaFileSystems::RandomAccessData>>> sources;std::vector<std::shared_ptr<const OpticalTrack>> tracks;std::int64_t logical=0;
        for(std::size_t index=0;index<descriptor.size();++index){checkCancellation(token);const auto& declaration=descriptor[index];const auto dataPath=utf8(paths[index]);auto found=std::find_if(sources.begin(),sources.end(),[&](const auto& entry){return Primitives::ordinalIdentifierEqualsIgnoreCase(entry.first,dataPath);});
            if(found==sources.end()){sources.emplace_back(dataPath,fileSource(paths[index]));found=std::prev(sources.end());}const auto source=found->second;
            if(source->length()>MaximumSigned||std::uint64_t(declaration.fileOffset)>source->length()||(source->length()-declaration.fileOffset)%declaration.sectorSize)throw MediaError(DiagnosticCode::GdiTrack);const auto available=std::int64_t((source->length()-declaration.fileOffset)/declaration.sectorSize);if(!available)throw MediaError(DiagnosticCode::GdiTrack);auto count=available;
            if(index+1<descriptor.size()){const auto declared=descriptor[index+1].lba-declaration.lba;if(declared<=0||declared>available)throw MediaError(DiagnosticCode::GdiTrack);count=declared;}
            OpticalTrackOptions value;value.sessionNumber=1;value.trackNumber=declaration.number;value.firstSector=declaration.lba;value.sectorCount=count;value.storedSectorSize=declaration.sectorSize;value.dataSource=source;value.sourceOffset=declaration.fileOffset;value.indexes.emplace_back(1,0);
            value.mode=declaration.control==AudioControl?OpticalTrackMode::Audio:declaration.sectorSize==DataBytes?OpticalTrackMode::Mode1Data2048:OpticalTrackMode::Mode1Raw2352;value.userDataLength=value.mode==OpticalTrackMode::Audio?declaration.sectorSize:DataBytes;if(value.mode==OpticalTrackMode::Mode1Raw2352)value.userDataOffset=RawUserOffset;
            logical=OpticalInternal::add(logical,OpticalInternal::multiply(count,value.userDataLength));tracks.push_back(std::make_shared<OpticalTrack>(std::move(value)));
        }
        auto options=context.source()->options();options.associatedPaths.clear();for(const auto& source:sources)options.associatedPaths.push_back(source.first);OpticalImageOptions image;image.logicalLength=logical;image.tracks=std::move(tracks);image.associatedFiles=std::vector<std::string>{context.source()->primaryPath()};image.associatedFiles->insert(image.associatedFiles->end(),options.associatedPaths.begin(),options.associatedPaths.end());
        std::map<std::string,std::string> metadata{{std::string(TrackCountKey),std::to_string(descriptor.size())},{std::string(ProfileKey),std::string(MediaProfile)}};for(const auto& declaration:descriptor){const auto prefix=std::string(TrackPrefix)+std::to_string(declaration.number);metadata[prefix+std::string(FileSuffix)]=declaration.fileName;metadata[prefix+std::string(LbaSuffix)]=std::to_string(declaration.lba);metadata[prefix+std::string(ControlSuffix)]=std::to_string(declaration.control);metadata[prefix+std::string(SectorSizeSuffix)]=std::to_string(declaration.sectorSize);}
        return std::make_shared<const MediaDocument>(std::make_shared<MediaSource>(std::move(options)),std::string(FormatId),MediaKind::Optical,std::make_shared<OpticalImage>(std::move(image)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::move(metadata));
    });
}
}
