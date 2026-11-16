#include "Reader.h"
#include "WaveSource.h"
#include "../../Common/Audio/Vorbis/PcmSource.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Models/Optical/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::BinCue {
namespace {
using namespace Constants;
std::filesystem::path pathOf(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
std::filesystem::path resolve(const std::filesystem::path& directory,std::string_view value){std::error_code error;const auto result=std::filesystem::absolute(directory/pathOf(value),error).lexically_normal();if(error)throw MediaError(DiagnosticCode::CueStructure);return result;}
void requireAudio(const FileDeclaration& file){if(file.kind()!=FileKind::Binary&&std::any_of(file.tracks().begin(),file.tracks().end(),[](const auto& track){return track.fields().mode!=OpticalTrackMode::Audio;}))throw MediaError(DiagnosticCode::CueStructure);}
std::shared_ptr<const MediaFileSystems::RandomAccessData> openFile(const FileDeclaration& file,const std::filesystem::path& path,std::stop_token token){
    requireAudio(file);auto source=fileSource(path);
    switch(file.kind()){case FileKind::Binary:return source;case FileKind::WavePcm:return wavePcmSource(std::move(source),true,token);case FileKind::Vorbis:{auto audio=Vorbis::openPcmSource(std::move(source),token);if(audio.channels!=CdChannels||audio.sampleRate!=CdRate)throw MediaError(DiagnosticCode::CueStructure);return audio.data;}}
    throw MediaError(DiagnosticCode::CueStructure);
}
std::int64_t indexOne(const TrackFields& track){const auto found=std::find_if(track.indexes.begin(),track.indexes.end(),[](const auto& index){return index.number()==1;});if(found==track.indexes.end())throw MediaError(DiagnosticCode::CueStructure);return found->relativeSector();}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{std::string(BinExtension),std::string(WaveExtension)},{MediaKind::Optical},{RepresentationKind::OpticalTracks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;if(!sameIdentifier(context.extension(),Constants::Extension))return false;
        try{const auto path=pathOf(context.source()->primaryPath());const auto sheet=readSheet(path,token);for(const auto& file:sheet.files()){checkCancellation(token);requireAudio(file);const auto resolved=resolve(path.parent_path(),file.path());std::error_code error;if(!std::filesystem::is_regular_file(resolved,error)||error)return false;if(file.kind()!=FileKind::Binary)openFile(file,resolved,token);}return true;}
        catch(const MediaFileSystems::MediaError& error){if(error.kind()==MediaFileSystems::ErrorKind::Cancelled)throw;return false;}
        catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto path=pathOf(context.source()->primaryPath());const auto sheet=readSheet(path,token);std::vector<std::shared_ptr<const OpticalTrack>> tracks;auto options=context.source()->options();options.associatedPaths.clear();std::int64_t firstSector=0,logical=0;
        for(const auto& file:sheet.files()){checkCancellation(token);const auto resolved=resolve(path.parent_path(),file.path());auto source=openFile(file,resolved,token);options.associatedPaths.push_back(utf8(resolved));const auto size=storedSize(file.tracks().front().fields().mode);
            if(!size||source->length()>MaximumSigned||source->length()<std::uint64_t(size)||(file.kind()!=FileKind::Vorbis&&source->length()%size))throw MediaError(DiagnosticCode::CueStructure);
            const auto sectorCount=std::int64_t(source->length()/size);std::vector<std::int64_t> starts;
            for(const auto& track:file.tracks()){if(storedSize(track.fields().mode)!=size)throw MediaError(DiagnosticCode::CueStructure);starts.push_back(indexOne(track.fields()));}
            for(std::size_t i=0;i<file.tracks().size();++i){const auto& fields=file.tracks()[i].fields();const auto start=starts[i],end=i+1<starts.size()?starts[i+1]:sectorCount;if(start<0||start>=sectorCount||end<=start||end>sectorCount)throw MediaError(DiagnosticCode::CueStructure);
                OpticalTrackOptions value;value.sessionNumber=1;value.trackNumber=fields.number;value.mode=fields.mode;value.firstSector=firstSector;value.sectorCount=end-start;value.storedSectorSize=size;value.userDataOffset=userOffset(fields.mode);value.userDataLength=userLength(fields.mode);value.dataSource=source;value.sourceOffset=OpticalInternal::multiply(start,size);
                for(const auto& index:fields.indexes){value.indexes.emplace_back(index.number(),index.relativeSector()-start);if(index.number()==0){if(index.relativeSector()>start)throw MediaError(DiagnosticCode::CueStructure);value.storedPregapSectors=start-index.relativeSector();if(value.storedPregapSectors)value.storedPregapSourceOffset=OpticalInternal::multiply(index.relativeSector(),size);}}
                value.pregapSectors=fields.pregap;value.postgapSectors=fields.postgap;value.flags=fields.flags;value.catalogNumber=sheet.catalog();value.isrc=fields.isrc;value.metadata=fields.metadata;
                firstSector=OpticalInternal::add(firstSector,value.sectorCount);logical=OpticalInternal::add(logical,OpticalInternal::multiply(value.sectorCount,value.userDataLength));tracks.push_back(std::make_shared<OpticalTrack>(std::move(value)));
            }
        }
        OpticalImageOptions image;image.logicalLength=logical;image.tracks=std::move(tracks);image.associatedFiles=std::vector<std::string>{context.source()->primaryPath()};image.associatedFiles->insert(image.associatedFiles->end(),options.associatedPaths.begin(),options.associatedPaths.end());
        std::map<std::string,std::string> metadata;for(const auto& [key,value]:sheet.metadata())if(!sameIdentifier(key,TrackCountKey)&&!sameIdentifier(key,FileCountKey))metadata.emplace(key,value);metadata[std::string(TrackCountKey)]=std::to_string(image.tracks->size());metadata[std::string(FileCountKey)]=std::to_string(sheet.files().size());
        return std::make_shared<const MediaDocument>(std::make_shared<MediaSource>(std::move(options)),std::string(FormatId),MediaKind::Optical,std::make_shared<OpticalImage>(std::move(image)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::move(metadata));
    });
}
}
