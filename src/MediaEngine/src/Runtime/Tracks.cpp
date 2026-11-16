#include "DiskState.h"
#include "SectorTracks.h"
#include "FluxConstants.h"
#include "../Encoding/SectorImageEncoder.h"
#include "../Codecs/Iso/Fm.h"
#include "../Codecs/Iso/Mfm.h"
#include "../Formats/ImageDisk/ImageDisk.h"
#include "../Formats/ImageDisk/Constants.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include "../Formats/HardDisk/Adaptec/Constants.h"
#include <limits>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeTrackWindow);}
std::int32_t dataRate(std::uint32_t cell,bool mfm){if(!cell)invalid();return static_cast<std::int32_t>(RuntimeDetails::FluxConstants::NanosecondsPerSecond/(std::uint64_t(cell)*(mfm?IsoFm::Constants::CellsPerDataBit:1)));}
MediaDiskSector field(const M::SectorBlock& b){MediaDiskSector result;result.index=static_cast<std::int64_t>(b.logicalBlock);result.data=b.data;const auto address=b.recordedAddress.value_or(b.address);result.cylinder=RuntimeDetails::coordinate(address.cylinder);result.head=RuntimeDetails::coordinate(address.head);result.number=RuntimeDetails::coordinate(address.number);result.integrityValid=b.integrityValid;result.recordedSizeCode=b.recordedSizeCode;result.headerIntegrityValid=b.headerIntegrityValid;result.dataIntegrityValid=b.dataIntegrityValid;result.deletedDataMark=b.deletedDataMark;result.recordedErrorCheckCode=b.recordedErrorCheckCode;return result;}
}
FluxTrackData RuntimeDisk::DecodedState::readTrack(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,const Encoding::Attributes& attributes){
    alive();if(cylinder<0||head<0||cylinder>=cylinders()||head>=heads()||!cell||!period)invalid();
    if(auto found=writtenTracks.find({cylinder,head});found!=writtenTracks.end())return found->second;
    if(document->formatId()==AcornWinchester::Constants::FormatId&&blocks())return readWinchesterTrack(cylinder,head,period);
    const auto pending=updated();const auto* representation=dynamic_cast<const SectorRepresentation*>(&pending->representation());if(!representation)invalid();
    const auto& image=representation->image();Encoding::SectorImageEncoder encoder;auto track=encoder.encodePhysicalTrack(image,cylinder,head,period);
    if(pending->metadata().contains(std::string(ImageDisk::Constants::MetadataKey))){
        const auto restored=ImageDisk::restore(representation->imageOwner(),pending->metadata());if(restored){
            const auto found=std::find_if(restored->tracks.begin(),restored->tracks.end(),[&](const auto& t){return t.cylinder==cylinder&&t.head==head;});
            if(found!=restored->tracks.end()){
                const auto mfm=found->mode>=ImageDisk::Mode::Mfm500;std::uint32_t rate=ImageDisk::Constants::Rate250;
                if(found->mode==ImageDisk::Mode::Fm500||found->mode==ImageDisk::Mode::Mfm500)rate=ImageDisk::Constants::Rate500;
                else if(found->mode==ImageDisk::Mode::Fm300||found->mode==ImageDisk::Mode::Mfm300)rate=ImageDisk::Constants::Rate300;
                const auto duration=RuntimeDetails::FluxConstants::NanosecondsPerSecond/(rate*(mfm?IsoFm::Constants::CellsPerDataBit:1));
                Encoding::Attributes physical=std::map<std::string,std::int32_t>{{std::string(mfm?IsoMfm::Constants::PhysicalAttribute:IsoFm::Constants::PhysicalAttribute),mfm?IsoMfm::Constants::PhysicalEnabled:IsoFm::Constants::PhysicalEnabled}};
                track=encoder.encodeTrack(image,cylinder,head,mfm?IsoMfm::Constants::Id:IsoFm::Constants::Id,physical,duration,period);
            }
        }
    }
    (void)encoding;(void)attributes;return FluxTrackData({FluxRevolutionData(period,std::move(track.revolution.intervals))});
}
void RuntimeDisk::DecodedState::writeTrackWindow(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,const Encoding::Attributes& attributes,bool ending){
    writable();if(document->formatId()==AcornWinchester::Constants::FormatId&&blocks()){
        if(encoding!=Adaptec::Constants::EncodingId)invalid();writeWinchesterWindow(cylinder,head,period,start,duration,offsets,ending);return;
    }
    if(encoding!=IsoFm::Constants::Id&&encoding!=IsoMfm::Constants::Id)invalid();const auto mfm=encoding==IsoMfm::Constants::Id;
    const auto source=readTrack(cylinder,head,encoding,cell,period,attributes);auto revolution=replaceFluxWindow(source.revolutions().front(),start,duration,offsets,ending);if(!duration)return;
    const Flux::Revolution flux{revolution.indexTimeNanoseconds(),std::vector<std::uint32_t>(revolution.intervalsNanoseconds().begin(),revolution.intervalsNanoseconds().end())};
    const auto decoded=mfm?IsoMfm::decode(flux):IsoFm::decode(flux,cell);std::vector<MediaDiskSector> fields;fields.reserve(decoded.sectors.size());
    for(std::size_t i=0;i<decoded.sectors.size();++i){const auto& s=decoded.sectors[i];MediaDiskSector f;f.index=static_cast<std::int64_t>(i);if(s.data)f.data=*s.data;f.cylinder=s.cylinder;f.head=s.head;f.number=s.number;f.integrityValid=s.integrityValid;f.recordedSizeCode=s.sizeCode;f.headerIntegrityValid=s.headerIntegrityValid;f.dataIntegrityValid=s.dataIntegrityValid;f.deletedDataMark=s.deletedDataMark;f.recordedErrorCheckCode=s.recordedErrorCheckCode;fields.push_back(std::move(f));}
    const TrackAddress address{cylinder,head};if(!trackTemplates.contains(address)&&sectors()){
        std::vector<M::SectorBlock> original;sectors()->visitAvailableBlocks([&](auto i){const auto a=sectors()->blockAddress(i);if(a&&a->cylinder==cylinder&&a->head==head)if(auto b=sectors()->block(i))original.push_back(*b);return true;});trackTemplates.emplace(address,std::move(original));
    }
    documentChanged=true;
    try{
        try{formatTrack(cylinder,head,fields,dataRate(cell,mfm),mfm);}
        catch(const MediaError&){
            const auto failure=std::current_exception();const auto pending=updated();const auto* image=dynamic_cast<const SectorRepresentation*>(&pending->representation());if(!image)throw;
            auto projected=RuntimeDetails::projectTrack(image->image(),cylinder,head,fields);
            document=std::make_shared<MediaDocument>(pending->mediaSource(),pending->formatId(),pending->mediaKind(),std::make_shared<SectorRepresentation>(std::move(projected)),std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>(pending->messages().begin(),pending->messages().end()),pending->metadata(),std::vector<std::string>(pending->diagnostics().begin(),pending->diagnostics().end()));
            changes.clear();dataMarks.clear();originals.clear();trackProjectionErrors.insert_or_assign(address,ProjectionError{cell,mfm,failure});
        }
    }catch(...){writtenTracks.insert_or_assign(address,FluxTrackData({revolution}));throw;}
    writtenTracks.insert_or_assign(address,FluxTrackData({std::move(revolution)}));
}
void RuntimeDisk::DecodedState::retryTrackProjection(){
    auto winchester=winchesterProjectionErrors;for(const auto& [address,error]:winchester){
        try{projectWinchesterTrack(address.first,address.second);}catch(const MediaError&){throw MediaError(DiagnosticCode::RuntimeTrackProjection);}winchesterProjectionErrors.erase(address);
    }
    const auto pendingErrors=trackProjectionErrors;for(const auto& [address,error]:pendingErrors){
        const auto pending=updated();const auto* image=dynamic_cast<const SectorRepresentation*>(&pending->representation());if(!image)throw MediaError(DiagnosticCode::RuntimeTrackProjection);
        std::vector<M::SectorBlock> blocks;image->image().visitAvailableBlocks([&](auto i){auto a=image->image().blockAddress(i);if(a&&a->cylinder==address.first&&a->head==address.second)if(auto b=image->image().block(i))blocks.push_back(*b);return true;});
        std::stable_sort(blocks.begin(),blocks.end(),[](const auto& a,const auto& b){return (a.recordedTrackOrder?static_cast<std::int64_t>(*a.recordedTrackOrder):static_cast<std::int64_t>(a.logicalBlock))<(b.recordedTrackOrder?static_cast<std::int64_t>(*b.recordedTrackOrder):static_cast<std::int64_t>(b.logicalBlock));});
        std::vector<MediaDiskSector> fields;for(const auto& block:blocks)fields.push_back(field(block));
        const auto raw=writtenTracks.find(address);const auto preserved=raw==writtenTracks.end()?std::optional<FluxTrackData>{}:std::optional(raw->second);
        try{formatTrack(address.first,address.second,fields,dataRate(error.cell,error.mfm),error.mfm);}
        catch(const MediaError&){if(preserved)writtenTracks.insert_or_assign(address,*preserved);throw MediaError(DiagnosticCode::RuntimeTrackProjection);}
        catch(...){if(preserved)writtenTracks.insert_or_assign(address,*preserved);throw;}
        if(preserved)writtenTracks.insert_or_assign(address,*preserved);
    }
}
FluxTrackData RuntimeDisk::readTrack(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,const Encoding::Attributes& attributes)const{std::lock_guard lock(state_->mutex);return state_->readTrack(cylinder,head,encoding,cell,period,attributes);}
void RuntimeDisk::writeTrackWindow(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,const Encoding::Attributes& attributes,bool ending){std::lock_guard lock(state_->mutex);state_->writeTrackWindow(cylinder,head,encoding,cell,period,start,duration,offsets,attributes,ending);}
}
