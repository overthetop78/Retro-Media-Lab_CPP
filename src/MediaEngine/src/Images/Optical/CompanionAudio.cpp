#include "CompanionAudio.h"
#include "Constants.h"
#include "../RecognitionContext.h"
#include "../../Formats/Optical/BinCue/WaveSource.h"
#include "../../Formats/Common/Audio/Vorbis/PcmSource.h"
#include "../../Models/Optical/Image.h"
#include "../../Models/Optical/Internal.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <filesystem>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace OpticalSessionConstants;
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
std::filesystem::path pathOf(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
std::filesystem::path name(const std::filesystem::path& stem,int format,int index){const auto variant=format%FormatsPerType;auto number=std::to_string(index);if(variant<PrefixCount&&number.size()<NumberWidth)number.insert(0,NumberWidth-number.size(),Zero);return pathOf(utf8(stem)+std::string(Prefixes[variant%PrefixCount])+number+std::string(format<FormatsPerType?OggExtension:WaveExtension));}
bool fileExists(const std::filesystem::path& path){std::error_code error;return std::filesystem::is_regular_file(path,error)&&!error;}
class PaddedData final:public MediaFileSystems::RandomAccessData {
public:
    PaddedData(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::uint64_t length):source_(std::move(source)),length_(length){}
    std::uint64_t length()const noexcept override{return length_;}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{checkCancellation(token);if(offset>length_||output.size()>length_-offset)throw MediaError(DiagnosticCode::OpticalSessionRange);const auto available=offset<source_->length()?std::size_t(std::min<std::uint64_t>(output.size(),source_->length()-offset)):0;if(available)source_->readExactly(offset,output.first(available),token);std::fill(output.begin()+available,output.end(),0);}
private:std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;std::uint64_t length_;
};
}
std::shared_ptr<const MediaDocument> attachOpticalCompanionAudio(std::shared_ptr<const MediaDocument> document,std::stop_token token){using namespace OpticalSessionConstants;if(!document)throw MediaError(DiagnosticCode::OpticalSessionImage);auto path=pathOf(document->mediaSource()->primaryPath());const auto suffix=utf8(path.extension());if(!sameIdentifier(suffix,IsoExtension)&&!sameIdentifier(suffix,BinExtension))return document;const auto* image=dynamic_cast<const OpticalImage*>(&document->representation());if(!image||!image->options().tracks||image->options().tracks->size()!=1||image->options().tracks->front()->isAudio())return document;path.replace_extension();const auto stem=path;
    int selected=-1,sequenceOffset=1;std::filesystem::path current;for(int format=0;format<FormatCount;++format){auto candidate=name(stem,format,1);if(fileExists(candidate)){selected=format;sequenceOffset=0;current=std::move(candidate);break;}candidate=name(stem,format,FirstAudioTrack);if(fileExists(candidate)){selected=format;current=std::move(candidate);break;}}if(selected<0)return document;
    auto tracks=*image->options().tracks;auto options=document->mediaSource()->options();auto end=OpticalInternal::add(OpticalInternal::add(tracks.front()->firstSector(),tracks.front()->sectorCount()),tracks.front()->options().postgapSectors);
    while(!current.empty()&&tracks.size()<MaximumTracks){checkCancellation(token);try{auto raw=fileSource(current);if(selected<FormatsPerType){auto pcm=Vorbis::openPcmSource(std::move(raw),token);if(pcm.channels!=Channels||pcm.sampleRate!=SampleRate)throw MediaError(DiagnosticCode::OpticalSessionImage);raw=std::move(pcm.data);}else raw=BinCue::wavePcmSource(std::move(raw),true,token);if(!raw->length())break;if(raw->length()>MaximumSourceBytes)throw MediaError(DiagnosticCode::OpticalSessionRange);
            auto count=OpticalInternal::add(std::int64_t(raw->length()),SectorBytes-1)/SectorBytes;std::int64_t storedPregap=0;std::array<std::uint8_t,ProbeBytes> probe{};if(raw->length()>=std::uint64_t(ProbeFrame)*SectorBytes+ProbeBytes){raw->readExactly(std::uint64_t(ProbeFrame)*SectorBytes,probe,token);if(count>PregapFrames&&qFromLittleEndian<std::uint32_t>(probe.data())==0)storedPregap=PregapFrames;}
            const auto padded=OpticalInternal::multiply(count,SectorBytes);auto source=std::make_shared<PaddedData>(std::move(raw),std::uint64_t(padded));const auto start=OpticalInternal::add(end,PregapFrames);count-=storedPregap;OpticalTrackOptions value;value.sessionNumber=1;value.trackNumber=int(tracks.size()+1);value.mode=OpticalTrackMode::Audio;value.firstSector=start;value.sectorCount=count;value.storedSectorSize=value.userDataLength=SectorBytes;value.dataSource=std::move(source);value.sourceOffset=OpticalInternal::multiply(storedPregap,SectorBytes);value.indexes={OpticalTrackIndex(0,-PregapFrames),OpticalTrackIndex(1,0)};value.pregapSectors=PregapFrames;value.storedPregapSectors=storedPregap;if(storedPregap)value.storedPregapSourceOffset=0;tracks.push_back(std::make_shared<OpticalTrack>(std::move(value)));options.associatedPaths.push_back(utf8(current));end=OpticalInternal::add(start,count);
        }catch(const MediaFileSystems::MediaError& error){if(error.kind()==MediaFileSystems::ErrorKind::Io||error.kind()==MediaFileSystems::ErrorKind::NotFound||error.kind()==MediaFileSystems::ErrorKind::Unsupported)break;throw;}
        current=name(stem,selected,int(tracks.size())+sequenceOffset);if(!fileExists(current))current.clear();
    }
    if(tracks.size()==1)return document;auto metadata=document->metadata();metadata[std::string(TrackCountKey)]=std::to_string(tracks.size());metadata[std::string(AudioCountKey)]=std::to_string(tracks.size()-1);OpticalImageOptions result=image->options();std::int64_t logical=0;for(const auto& track:tracks)logical=OpticalInternal::add(logical,OpticalInternal::multiply(track->sectorCount(),track->userDataLength()));result.logicalLength=logical;result.tracks=std::move(tracks);result.associatedFiles=std::vector<std::string>{document->mediaSource()->primaryPath()};result.associatedFiles->insert(result.associatedFiles->end(),options.associatedPaths.begin(),options.associatedPaths.end());return std::make_shared<const MediaDocument>(std::make_shared<MediaSource>(std::move(options)),document->formatId(),document->mediaKind(),std::make_shared<OpticalImage>(std::move(result)),std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),std::move(metadata),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
}
}
