#include "DiskState.h"
#include "../Sources/DriveAccess.h"
#include "../Formats/CpcDsk/Constants.h"
#include "../Formats/ImageDisk/Constants.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
M::SectorAddress RuntimeDisk::State::address(std::int64_t index)const{if(heads()>0&&sectorsPerTrack()>0)return {RuntimeDetails::coordinate(index/(std::int64_t(heads())*sectorsPerTrack())),index/sectorsPerTrack()%heads(),index%sectorsPerTrack()};return {-1,-1,-1};}
std::shared_ptr<const M::SectorBlock> RuntimeDisk::State::original(std::int64_t index,std::stop_token token)const{
    if(const auto found=originals.find(index);found!=originals.end())return found->second;
    return sectors()?sectors()->block(static_cast<std::uint64_t>(index),token):nullptr;
}
std::optional<std::int64_t> RuntimeDisk::findSector(std::int32_t cylinder,std::int32_t head,std::int32_t number)const{
    std::lock_guard lock(state_->mutex);state_->alive();
    if(const auto* image=state_->sectors()){
        std::optional<std::int64_t> result;image->visitAvailableBlocks([&](auto index){if(image->blockAddress(index)==M::SectorAddress{cylinder,head,number}){result=static_cast<std::int64_t>(index);return false;}return true;});return result;
    }
    if(cylinder<0||cylinder>=state_->cylinders()||head<0||head>=state_->heads()||number<0||number>=state_->sectorsPerTrack())return {};
    return (std::int64_t(cylinder)*state_->heads()+head)*state_->sectorsPerTrack()+number;
}
std::vector<std::int64_t> RuntimeDisk::trackSectors(std::int32_t cylinder,std::int32_t head)const{
    std::lock_guard lock(state_->mutex);state_->alive();std::vector<std::int64_t> result;
    if(const auto* image=state_->sectors()){
        std::vector<std::pair<std::int64_t,std::int64_t>> order;
        image->visitAvailableBlocks([&](auto index){const auto address=image->blockAddress(index);if(address&&address->cylinder==cylinder&&address->head==head){const auto block=image->block(index);order.emplace_back(block->recordedTrackOrder?static_cast<std::int64_t>(*block->recordedTrackOrder):static_cast<std::int64_t>(index),static_cast<std::int64_t>(index));}return true;});
        std::stable_sort(order.begin(),order.end(),[](const auto& a,const auto& b){return a.first<b.first;});for(const auto& value:order)result.push_back(value.second);return result;
    }
    if(cylinder<0||cylinder>=state_->cylinders()||head<0||head>=state_->heads())return result;
    for(int i=0;i<state_->sectorsPerTrack();++i)result.push_back((std::int64_t(cylinder)*state_->heads()+head)*state_->sectorsPerTrack()+i);return result;
}
MediaDiskSector RuntimeDisk::State::read(std::int64_t index,std::stop_token token){
    validate(index);checkCancellation(token);const auto physical=sectors()?sectors()->blockAddress(static_cast<std::uint64_t>(index)).value_or(address(index)):address(index);
    DriveAccessScope access(document->mediaSource()->options().drive.get(),RuntimeDetails::position(index,physical),false);
    const auto block=original(index,token);const auto recorded=block?block->recordedAddress.value_or(physical):physical;
    MediaDiskSector result;result.index=index;result.cylinder=RuntimeDetails::coordinate(recorded.cylinder);result.head=RuntimeDetails::coordinate(recorded.head);result.number=RuntimeDetails::coordinate(recorded.number);
    if(block){result.integrityValid=block->integrityValid;result.diagnosticCode=block->diagnosticCode;result.recordedErrorCheckCode=block->recordedErrorCheckCode;result.recordedDataPrefix=block->recordedDataPrefix;result.headerIntegrityValid=block->headerIntegrityValid;result.dataIntegrityValid=block->dataIntegrityValid;result.deletedDataMark=block->deletedDataMark;result.recordedSizeCode=block->recordedSizeCode;}
    if(const auto changed=changes.find(index);changed!=changes.end()){
        result.data=changed->second;result.integrityValid=!block||block->headerIntegrityValid!=false;result.dataIntegrityValid=true;result.diagnosticCode.reset();result.recordedErrorCheckCode.clear();result.recordedDataPrefix.clear();
        if(const auto mark=dataMarks.find(index);mark!=dataMarks.end())result.deletedDataMark=mark->second;
    }else if(block)result.data=block->data;
    else {if(!blocks())throw MediaError(DiagnosticCode::RuntimeDiskUnavailable,{std::to_string(index)});result.data.resize(static_cast<std::size_t>(size()));blocks()->readExactly(index*size(),result.data,token);}
    access.finish();return result;
}
void RuntimeDisk::State::write(std::int64_t index,std::span<const std::uint8_t> bytes,std::optional<bool> mark){
    validate(index);writable();
    if(mark==true&&(!sectors()||(document->formatId()!=CpcDsk::Constants::FormatId&&!document->metadata().contains(std::string(ImageDisk::Constants::MetadataKey)))))throw MediaError(DiagnosticCode::RuntimeDiskSector);
    const auto block=original(index);const auto expected=block?block->data.size():static_cast<std::size_t>(size());if(bytes.size()!=expected)throw MediaError(DiagnosticCode::RuntimeDiskSector);
    const auto physical=block?block->address:address(index);DriveAccessScope access(document->mediaSource()->options().drive.get(),RuntimeDetails::position(index,physical),true);
    auto copy=std::vector<std::uint8_t>(bytes.begin(),bytes.end());
    if(block)originals[index]=block;changes[index]=std::move(copy);if(mark)dataMarks[index]=*mark;access.finish();
}
}
