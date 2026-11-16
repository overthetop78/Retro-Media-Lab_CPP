#include "Reader.h"
#include "../../../Sources/AtomicImageSetWriter.h"
#include "../../../Models/Optical/Internal.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::BinCue {
namespace {
using namespace Constants;
std::string normalized(std::string_view value){if(value.empty()||value.front()==BinExtension.front())return std::string(value);return std::string(BinExtension.substr(0,1))+std::string(value);}
void copy(const OpticalTrack& track,std::int64_t offset,std::int64_t sectors,const ImageOutput& output,std::stop_token token){const auto& value=track.options();auto remaining=OpticalInternal::multiply(sectors,value.storedSectorSize);std::vector<std::uint8_t> buffer(TransferBytes);while(remaining){checkCancellation(token);const auto count=std::size_t(std::min(remaining,std::int64_t(buffer.size())));auto bytes=std::span(buffer).first(count);value.dataSource->readExactly(std::uint64_t(offset),bytes,token);output(bytes);offset=OpticalInternal::add(offset,count);remaining-=count;}}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::OpticalTracks},true};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{using namespace Constants;const auto* image=dynamic_cast<const OpticalImage*>(&document.representation());if(!supportsTarget(format,normalized(extension))||document.mediaKind()!=MediaKind::Optical||!image||!image->options().tracks||image->options().tracks->empty()||image->options().layerCount||image->options().faceCount)return false;
    std::unordered_set<int> numbers;const auto catalog=image->options().tracks->front()->options().catalogNumber;
    for(const auto& track:*image->options().tracks){const auto& value=track->options();if(!numbers.insert(value.trackNumber).second||value.sessionNumber!=1||value.subchannelSource||!storedSize(value.mode)||value.storedSectorSize!=storedSize(value.mode)||value.catalogNumber!=catalog||(value.storedPregapSectors&&!value.storedPregapSourceOffset))return false;}return true;
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{using namespace Constants;checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto ext=path.extension().u8string();if(!Writer{}.canWrite(*document,format,std::string(ext.begin(),ext.end())))throw MediaError(DiagnosticCode::CueOutput);std::error_code error;const auto descriptor=std::filesystem::absolute(path,error).lexically_normal();if(error)throw MediaError(DiagnosticCode::CueOutput);const auto rawStem=descriptor.stem().u8string();const std::string stem(rawStem.begin(),rawStem.end());
        const auto& image=dynamic_cast<const OpticalImage&>(document->representation());std::vector<ImageFileOutput> outputs;std::vector<std::filesystem::path> paths;std::vector<FileDeclaration> files;
        for(const auto& track:*image.options().tracks){checkCancellation(token);const auto& value=track->options();auto number=std::to_string(track->trackNumber());if(number.size()<TrackNumberWidth)number.insert(0,TrackNumberWidth-number.size(),Zero);const auto name=stem+std::string(OutputTrack)+number+std::string(BinExtension);const auto destination=descriptor.parent_path()/std::filesystem::path(std::u8string(name.begin(),name.end()));
            outputs.push_back({destination,[track](const auto& write,auto stop){const auto& v=track->options();if(v.storedPregapSectors)copy(*track,*v.storedPregapSourceOffset,v.storedPregapSectors,write,stop);copy(*track,v.sourceOffset,v.sectorCount,write,stop);}});paths.push_back(destination);
            TrackFields fields;fields.number=value.trackNumber;fields.mode=value.mode;if(value.storedPregapSectors)fields.indexes.emplace_back(0,0);fields.indexes.emplace_back(1,value.storedPregapSectors);for(const auto& index:value.indexes)if(index.number()>1)fields.indexes.emplace_back(index.number(),OpticalInternal::add(value.storedPregapSectors,index.relativeSector()));fields.flags=value.flags;fields.pregap=value.pregapSectors;fields.postgap=value.postgapSectors;fields.isrc=value.isrc;fields.metadata=value.metadata;files.emplace_back(name,FileKind::Binary,std::vector<TrackDeclaration>{TrackDeclaration(std::move(fields))});
        }
        OpticalMetadata metadata;for(const auto& [key,value]:document->metadata())if(sameIdentifier(key,Title)||sameIdentifier(key,Performer)||sameIdentifier(key,Songwriter)||Primitives::ordinalIdentifierStartsWithIgnoreCase(key,RemPrefix))metadata.emplace(key,value);
        const Sheet sheet(std::move(files),image.options().tracks->front()->options().catalogNumber,std::move(metadata));auto text=serializeSheet(sheet,token);outputs.push_back({descriptor,[text=std::move(text)](const auto& write,auto stop){checkCancellation(stop);write(std::span(reinterpret_cast<const std::uint8_t*>(text.data()),text.size()));}});paths.push_back(descriptor);writeAtomicImageSet(outputs,token);return paths;
    });
}
}
