#include <RetroMediaLab/MediaEngine/LinearVisualization.h>
#include "Providers.h"
#include "Constants.h"
#include "../Models/Blocks/BlockImage.h"
#include "../Models/Optical/Image.h"
#include <RetroMediaLab/MediaFileSystems/VolumeConstants.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
LinearVisualizationModel describeLinearVisualization(std::shared_ptr<const MediaDocument> document,const VisualizationDescriptor& descriptor,std::stop_token token){
    if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);
    LinearVisualizationModel result;
    const auto blocks=dynamic_cast<const BlockImage*>(&document->representation());
    const auto optical=dynamic_cast<const OpticalImage*>(&document->representation());
    if(!blocks && !optical)throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Descriptor)});
    if(blocks){result.logicalLength=blocks->logicalBlockCount();result.blockSize=blocks->logicalBlockSize();result.geometry=blocks->geometry();result.cartridge=document->mediaKind()==MediaKind::Cartridge;}
    else {const auto& options=optical->options();result.logicalLength=descriptor.fields().elements.empty()?options.logicalLength.value_or(0):0;result.faces=options.faceCount;result.layers=options.layerCount;if(options.associatedFiles)result.associatedFiles=*options.associatedFiles;}
    for(const auto& element:descriptor.fields().elements){
        checkCancellation(token);const auto& fields=element.fields();LinearVisualizationItem item;
        if(blocks && (fields.position>blocks->logicalBlockCount() || fields.length.value_or(1)>blocks->logicalBlockCount()-fields.position))throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Length)});
        const auto start=blocks?fields.position*result.blockSize:fields.position;
        const auto length=blocks?fields.length.value_or(1)*result.blockSize:fields.length.value_or(1);
        for(const auto& volume:document->volumeInfos())if(start>=volume.start && length<=volume.length && start-volume.start<=volume.length-length){item.volume=volume;break;}
        if(blocks){
            const auto found=std::find_if(blocks->ranges().begin(),blocks->ranges().end(),[&](const auto& range){return start>=range.address() && length<=range.length() && start-range.address()<=range.length()-length;});
            if(found!=blocks->ranges().end() && found->kind()!=DataRangeKind::Unavailable){item.blockState=BlockVisualizationState::Available;if(item.volume && item.volume->partitionTable && (*item.volume->partitionTable==MediaFileSystems::VolumeConstants::Mbr || *item.volume->partitionTable==MediaFileSystems::VolumeConstants::Gpt))item.blockState=BlockVisualizationState::Allocated;}
        }else if(optical->options().tracks){
            for(const auto& track:*optical->options().tracks)if(track->sessionNumber()==fields.sessionNumber && track->trackNumber()==fields.trackNumber){const auto& options=track->options();item.opticalKind=options.dataSource?(track->isAudio()?OpticalVisualizationKind::Audio:OpticalVisualizationKind::Data):OpticalVisualizationKind::Unknown;item.pregap=options.pregapSectors;item.postgap=options.postgapSectors;item.storedSectorSize=options.storedSectorSize;break;}
        }
        result.logicalLength=std::max(result.logicalLength,fields.position+fields.length.value_or(1));result.items.push_back(std::move(item));
    }return result;
}
std::future<std::vector<std::uint8_t>> readVisualizationPreviewAsync(std::shared_ptr<const MediaDocument> document,VisualizationElement element,std::stop_token token){
    return std::async(std::launch::async,[document=std::move(document),element=std::move(element),token]{
        if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);checkCancellation(token);const auto& fields=element.fields();
        if(const auto blocks=dynamic_cast<const BlockImage*>(&document->representation())){
            if(fields.position>blocks->logicalBlockCount() || fields.length.value_or(1)>blocks->logicalBlockCount()-fields.position)throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Length)});
            const auto address=fields.position*blocks->logicalBlockSize();const auto length=std::min<std::int64_t>(Visualization::Constants::PreviewBytes,fields.length.value_or(1)*blocks->logicalBlockSize());std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));blocks->readExactly(address,bytes,token);return bytes;
        }
        const auto& optical=Visualization::representation<OpticalImage>(*document);
        if(optical.options().tracks)for(const auto& track:*optical.options().tracks)if(track->sessionNumber()==fields.sessionNumber && track->trackNumber()==fields.trackNumber)return track->readStoredSectorAsync(0,token).get();
        throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Track)});
    });
}
}
