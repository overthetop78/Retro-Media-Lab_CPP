#include "PhysicalStates.h"
#include "../Formats/HardDisk/Acorn/Descriptor.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include "../Formats/HardDisk/Adaptec/Codec.h"
#include "../Formats/HardDisk/Adaptec/Constants.h"
#include <QString>
#include <array>
namespace RetroMediaLab::MediaEngine {
namespace A=AcornWinchester::Constants;namespace C=Adaptec::Constants;namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeTrackProjection);}
Flux::NanosecondRevolution neutral(const FluxRevolutionData& r){return {r.indexTimeNanoseconds(),std::vector<std::uint32_t>(r.intervalsNanoseconds().begin(),r.intervalsNanoseconds().end())};}
}
RuntimeDisk::MfmState::MfmState(std::shared_ptr<RuntimeDetails::Services> engine,std::shared_ptr<const MediaDocument> source,bool allowed,bool owns):State(std::move(engine),std::move(source),allowed,owns),representation(std::dynamic_pointer_cast<const MfmEmu::Representation>(document->representationOwner())){if(!representation)throw MediaError(DiagnosticCode::RuntimeDiskRequest);}
RuntimeDisk::MfmState::~MfmState()noexcept{try{close();}catch(...) {}}
std::optional<std::int64_t> RuntimeDisk::MfmState::findSector(std::int32_t cylinder,std::int32_t head,std::int32_t number)const{alive();return representation->sectorReader()->findSector(cylinder,head,number);}
std::vector<std::int64_t> RuntimeDisk::MfmState::trackSectors(std::int32_t cylinder,std::int32_t head)const{alive();return representation->sectorReader()->trackSectors(cylinder,head);}
MediaDiskSector RuntimeDisk::MfmState::read(std::int64_t index,std::stop_token token){
    alive();const auto b=representation->sectorReader()->readSector(index,token);MediaDiskSector result;result.index=static_cast<std::int64_t>(b.logicalBlock);result.data=b.data;
    result.cylinder=RuntimeDetails::coordinate(b.address.cylinder);result.head=RuntimeDetails::coordinate(b.address.head);result.number=RuntimeDetails::coordinate(b.address.number);result.integrityValid=b.integrityValid;result.diagnosticCode=b.diagnosticCode;result.recordedErrorCheckCode=b.recordedErrorCheckCode;result.recordedDataPrefix=b.recordedDataPrefix;result.headerIntegrityValid=b.headerIntegrityValid;result.dataIntegrityValid=b.dataIntegrityValid;result.deletedDataMark=b.deletedDataMark;result.recordedSizeCode=b.recordedSizeCode;return result;
}
void RuntimeDisk::MfmState::write(std::int64_t index,std::span<const std::uint8_t> bytes,std::optional<bool> mark){
    writable();if(mark==true||bytes.empty()||bytes.size()!=static_cast<std::size_t>(size()))throw MediaError(DiagnosticCode::RuntimeDiskSector);
    const auto sector=read(index,{});const auto track=readTrack(sector.cylinder,sector.head,C::EncodingId,C::CellNanoseconds,C::NominalIndexNanoseconds,{});
    const auto window=Adaptec::createSectorWrite(neutral(track.revolutions().front()),sector.cylinder,sector.head,index,bytes);
    writeTrackWindow(sector.cylinder,sector.head,C::EncodingId,C::CellNanoseconds,C::NominalIndexNanoseconds,window.startNanoseconds,window.durationNanoseconds,window.transitions,{},false);
}
FluxTrackData RuntimeDisk::MfmState::readTrack(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,const Attributes& attributes){
    alive();if(!cell)throw MediaError(DiagnosticCode::RuntimeTrackWindow);(void)encoding;(void)attributes;const auto address=std::tuple{cylinder,head,period};if(cached&&cachedAddress==address)return *cached;
    const auto track=representation->readTrack(cylinder,head,period);auto result=FluxTrackData({FluxRevolutionData(track.indexTime,track.intervals)});cached=result;cachedAddress=address;return result;
}
void RuntimeDisk::MfmState::writeTrackWindow(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,const Attributes& attributes,bool ending){
    writable();if(!cell)throw MediaError(DiagnosticCode::RuntimeTrackWindow);(void)encoding;(void)attributes;
    auto replacement=representation->withTrackWindow(cylinder,head,period,start,duration,offsets,ending);if(!duration)return;representation=std::move(replacement);cached.reset();changed=true;
}
void RuntimeDisk::MfmState::updateMetadata(const std::map<std::string,std::string>& metadata){
    alive();if(metadata.empty())return;writable();const auto found=metadata.find(std::string(A::DescriptorKey));if(metadata.size()!=1||found==metadata.end())invalid();
    const auto descriptor=AcornWinchester::decodeDescriptor(found->second);std::array<std::uint8_t,C::ParameterFlagCount> flags{};if(!AcornWinchester::tryWritePhysicalParameters(descriptor,representation->capacity(),flags))invalid();
    const auto track=readTrack(0,0,C::EncodingId,C::CellNanoseconds,C::NominalIndexNanoseconds,{});const auto fields=Adaptec::decode(neutral(track.revolutions().front()),0,0,size());std::size_t anchor=fields.size();
    for(std::size_t i=0;i<fields.size();++i)if(fields[i].sector.address.number==0&&fields[i].sector.headerIntegrityValid==true){anchor=i;break;}
    if(anchor==fields.size()||fields.size()<flags.size())invalid();
    for(std::size_t i=0;i<flags.size();++i){const auto& f=fields[(anchor+i)%fields.size()];if(f.sector.headerIntegrityValid!=true||f.id.size()<=C::FlagsOffset||(f.id[C::FlagsOffset]&C::DefectCountMask)!=flags[i])invalid();}
}
void RuntimeDisk::MfmState::prepareBlockFormat(const HardDiskGeometry& geometry){
    writable();if(!AcornWinchester::supportsGeometry(geometry)||geometry.cylinders()>cylinders()||geometry.heads()>heads())invalid();
}
void RuntimeDisk::MfmState::flush(std::stop_token token){
    alive();checkCancellation(token);if(!changed)return;auto options=document->mediaSource()->options();options.drive.reset();
    auto pending=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(options)),document->formatId(),document->mediaKind(),representation,std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
    services->writing.writingService().writeAsync(pending,std::filesystem::path(QString::fromUtf8(document->mediaSource()->primaryPath()).toStdU16String()),document->formatId(),token).get();
    options=document->mediaSource()->options();options.knownLength.reset();options.requestedFormatId=document->formatId();auto reopened=services->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
    auto image=std::dynamic_pointer_cast<const MfmEmu::Representation>(reopened->representationOwner());if(!image)invalid();representation=std::move(image);document=std::move(reopened);changed=false;
}
}
