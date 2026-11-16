#include "DiskState.h"
#include "SectorTracks.h"
#include "../Formats/ImageDisk/ImageDisk.h"
#include "../Formats/ImageDisk/Constants.h"
#include "../Formats/CpcDsk/Constants.h"
#include "../Models/Sectors/SizeCodes.h"
#include "../Models/Sectors/SectorConstants.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeDiskSector);}
std::uint8_t byte(std::int32_t value){if(value<0||value>ImageDisk::Constants::MaxByte)invalid();return static_cast<std::uint8_t>(value);}
ImageDisk::Mode mode(std::optional<std::int32_t> rate,std::optional<bool> mfm,const ImageDisk::Track* old){
    using namespace ImageDisk;if(!rate&&!mfm&&old)return old->mode;if(!rate||!mfm)invalid();
    switch(*rate){
    case Constants::Rate500:return *mfm?Mode::Mfm500:Mode::Fm500;
    case Constants::Rate300:return *mfm?Mode::Mfm300:Mode::Fm300;
    case Constants::Rate250:return *mfm?Mode::Mfm250:Mode::Fm250;
    default:invalid();}
}
}
void RuntimeDisk::DecodedState::formatTrack(std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector> fields,std::optional<std::int32_t> dataRate,std::optional<bool> mfm){
    writable();const auto pending=this->updated();
    const auto* representation=dynamic_cast<const SectorRepresentation*>(&pending->representation());if(!representation)invalid();
    auto previous=representation->imageOwner();RuntimeDetails::validateTrack(*previous,cylinder,head,fields);
    auto metadata=pending->metadata();std::shared_ptr<const M::SectorImage> updated;
    if(metadata.contains(std::string(ImageDisk::Constants::MetadataKey))){
        auto original=ImageDisk::restore(previous,metadata);if(!original)invalid();
        const auto found=std::find_if(original->tracks.begin(),original->tracks.end(),[&](const auto& t){return t.cylinder==cylinder&&t.head==head;});
        ImageDisk::Track track;track.mode=mode(dataRate,mfm,found==original->tracks.end()?nullptr:&*found);track.cylinder=byte(cylinder);track.head=byte(head);
        for(const auto& f:fields){
            if(f.headerIntegrityValid==false||f.data.size()>ImageDisk::Constants::MaxSize)invalid();
            if(f.recordedSizeCode&&(*f.recordedSizeCode>SectorConstants::MaximumIsoSizeCode||sectorSizeBytes(*f.recordedSizeCode)!=f.data.size()))invalid();
            ImageDisk::Sector sector;sector.cylinder=byte(f.cylinder);sector.head=byte(f.head);sector.number=byte(f.number);sector.size=static_cast<std::int32_t>(f.data.size());
            sector.recordType=ImageDisk::recordType(false,f.integrityValid!=false,f.deletedDataMark==true);sector.data=f.data;track.sectors.push_back(std::move(sector));
        }
        if(found==original->tracks.end())original->tracks.push_back(std::move(track));else *found=std::move(track);
        updated=ImageDisk::buildImage(original->tracks);original->sectorImage=updated;for(auto& [key,value]:ImageDisk::metadata(*original))metadata[key]=std::move(value);
    }else {
        const auto found=trackTemplates.find({cylinder,head});std::optional<std::span<const M::SectorBlock>> original;
        if(found!=trackTemplates.end())original=std::span<const M::SectorBlock>(found->second);
        updated=RuntimeDetails::replaceTrack(*previous,cylinder,head,fields,pending->formatId()!=CpcDsk::Constants::FormatId,original);
    }
    const auto formatId=updated->geometry().formatId;
    auto replacement=std::make_shared<MediaDocument>(pending->mediaSource(),formatId,pending->mediaKind(),std::make_shared<SectorRepresentation>(std::move(updated)),std::vector<M::VolumeDescriptor>{},
        std::vector<M::MessageRecipe>(pending->messages().begin(),pending->messages().end()),std::move(metadata),std::vector<std::string>(pending->diagnostics().begin(),pending->diagnostics().end()));
    document=std::move(replacement);changes.clear();dataMarks.clear();originals.clear();documentChanged=true;
    writtenTracks.erase({cylinder,head});trackProjectionErrors.erase({cylinder,head});trackTemplates.erase({cylinder,head});
}
void RuntimeDisk::formatTrack(std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector> fields,std::optional<std::int32_t> rate,std::optional<bool> mfm){std::lock_guard lock(state_->mutex);state_->formatTrack(cylinder,head,fields,rate,mfm);}

}
