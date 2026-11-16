#include "Container.h"
#include "../../../Sources/AtomicImageSetWriter.h"
#include "../../../Models/Optical/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Gdi {
namespace {
using namespace Constants;
int sectorSize(const OpticalTrack& track){const auto& value=track.options();switch(value.mode){case OpticalTrackMode::Audio:case OpticalTrackMode::Mode1Raw2352:return value.storedSectorSize==RawBytes?RawBytes:0;case OpticalTrackMode::Mode1Data2048:return value.storedSectorSize==DataBytes?DataBytes:0;default:return 0;}}
std::string normalized(std::string_view extension){if(extension.empty()||extension.front()==ExtensionSeparator)return std::string(extension);return std::string(1,ExtensionSeparator)+std::string(extension);}
void copyTrack(const OpticalTrack& track,const ImageOutput& output,int size,std::stop_token token){const auto& value=track.options();const auto total=OpticalInternal::multiply(value.sectorCount,size);std::vector<std::uint8_t> buffer(TransferBytes);std::int64_t copied=0;while(copied<total){checkCancellation(token);const auto count=std::size_t(std::min(std::int64_t(buffer.size()),total-copied));auto bytes=std::span(buffer).first(count);value.dataSource->readExactly(std::uint64_t(OpticalInternal::add(value.sourceOffset,copied)),bytes,token);output(bytes);copied+=count;}}
std::vector<std::shared_ptr<const OpticalTrack>> ordered(const OpticalImage& image){auto tracks=*image.options().tracks;std::sort(tracks.begin(),tracks.end(),[](const auto& a,const auto& b){return a->trackNumber()<b->trackNumber();});return tracks;}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::OpticalTracks},true};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{
    const auto* image=dynamic_cast<const OpticalImage*>(&document.representation());if(!supportsTarget(format,normalized(extension))||document.mediaKind()!=MediaKind::Optical||!image||!image->options().tracks||image->options().tracks->empty()||image->options().layerCount||image->options().faceCount)return false;
    const auto tracks=ordered(*image);std::unordered_set<int> numbers;for(std::size_t i=0;i<tracks.size();++i){const auto& track=*tracks[i];const auto& value=track.options();if(!numbers.insert(track.trackNumber()).second||track.firstSector()<0||value.storedPregapSectors||value.subchannelSource||sectorSize(track)<=0||track.sectorCount()<=0)return false;
        if(i+1<tracks.size()){const auto next=tracks[i+1]->firstSector();if(next<=track.firstSector()||next-track.firstSector()!=track.sectorCount())return false;}}
    return true;
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{using namespace Constants;if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto ext=path.extension().u8string();if(!Writer{}.canWrite(*document,format,std::string(ext.begin(),ext.end())))throw MediaError(DiagnosticCode::GdiWrite);std::error_code error;const auto descriptor=std::filesystem::absolute(path,error).lexically_normal();if(error)throw MediaError(DiagnosticCode::GdiWrite);const auto rawStem=descriptor.stem().u8string();const std::string stem(rawStem.begin(),rawStem.end());if(stem.find(Quote)!=std::string::npos)throw MediaError(DiagnosticCode::GdiWrite);
        const auto& image=dynamic_cast<const OpticalImage&>(document->representation());const auto tracks=ordered(image);std::vector<ImageFileOutput> files;std::vector<std::filesystem::path> paths;std::string text=std::to_string(tracks.size())+std::string(NewLine);
        for(const auto& track:tracks){checkCancellation(token);auto number=std::to_string(track->trackNumber());if(number.size()<TrackNumberWidth)number.insert(0,TrackNumberWidth-number.size(),Zero);const auto name=stem+std::string(OutputTrack)+number+std::string(BinExtension);const auto output=descriptor.parent_path()/std::filesystem::path(std::u8string(name.begin(),name.end()));const auto size=sectorSize(*track),control=track->isAudio()?AudioControl:DataControl;
            files.push_back({output,[track,size](const auto& write,auto stop){copyTrack(*track,write,size,stop);}});paths.push_back(output);
            text+=std::to_string(track->trackNumber())+Separator+std::to_string(track->firstSector())+Separator+std::to_string(control)+Separator+std::to_string(size)+Separator+Quote+name+Quote+Separator+Zero+std::string(NewLine);}
        files.push_back({descriptor,[text=std::move(text)](const auto& write,auto stop){checkCancellation(stop);write(std::span(reinterpret_cast<const std::uint8_t*>(text.data()),text.size()));}});paths.push_back(descriptor);writeAtomicImageSet(files,token);return paths;});
}
}
