#include "Representation.h"
#include "Signal.h"
#include "../Adaptec/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::MfmEmu {
Representation::Representation(std::shared_ptr<const Container> container):container_(std::move(container)){if(!container_)throw MediaError(DiagnosticCode::MfmHeader);}
std::shared_ptr<const Representation> Representation::createUnformatted(std::int32_t c,std::int32_t h,std::int32_t bytes,std::uint32_t rate,std::uint32_t start){return std::make_shared<const Representation>(Container::blank(Header::create(c,h,bytes,rate,start)));}
std::shared_ptr<Adaptec::SectorReader> Representation::sectors()const{
    std::lock_guard lock(mutex_);if(!sectors_){auto container=container_;sectors_=std::make_shared<Adaptec::SectorReader>([container](std::int32_t c,std::int32_t h){const auto& header=container->header();std::vector<std::uint8_t> bytes(static_cast<std::size_t>(header.trackDataLength()));container->readTrackBytes(c,h,0,bytes);return std::vector<Flux::NanosecondRevolution>{readSignal(bytes,header.sampleRate(),header.startNanoseconds(),Adaptec::Constants::NominalIndexNanoseconds)};},cylinders(),heads());}return sectors_;
}
std::optional<std::int64_t> Representation::logicalLength()const{return capacity();}
std::int64_t Representation::capacity()const{return sectors()->capacity();}
std::int32_t Representation::logicalBlockSize()const{return sectors()->sectorSize();}
std::int64_t Representation::logicalBlockCount()const{return sectors()->sectorCount();}
std::future<void> Representation::readExactlyAsync(std::int64_t offset,MediaFileSystems::MutableBytes bytes,std::stop_token token)const{return std::async(std::launch::deferred,[reader=sectors(),offset,bytes,token]{reader->readExactly(offset,bytes,token);});}
std::shared_ptr<const MediaDocument> Representation::createLogicalView(std::shared_ptr<const MediaDocument> document)const{
    if(!document||document->representationOwner().get()!=this)throw MediaError(DiagnosticCode::MfmLogicalView);const auto reader=sectors();const auto length=reader->capacity();if(!length)return {};const auto recorded=reader->geometry();HardDiskGeometry shape(recorded.cylinders,recorded.heads,recorded.sectorsPerTrack,recorded.sectorSize);auto blocks=std::make_shared<const BlockImage>(length,recorded.sectorSize,std::vector<DataRange>{DataRange(0,length,DataRangeKind::Stored,Adaptec::blockData(reader))},shape.capacity()==length?std::optional(shape):std::nullopt);
    return std::make_shared<const MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
}
std::shared_ptr<const Representation> Representation::withTrackBytes(std::int32_t c,std::int32_t h,std::span<const std::uint8_t> bytes)const{return std::make_shared<const Representation>(container_->withTrackBytes(c,h,bytes));}
Flux::NanosecondRevolution Representation::readTrack(std::int32_t c,std::int32_t h,std::uint32_t period,std::stop_token token)const{std::vector<std::uint8_t> bytes(static_cast<std::size_t>(trackByteLength()));readTrackBytes(c,h,0,bytes,token);return readSignal(bytes,sampleRate(),startNanoseconds(),period,token);}
std::shared_ptr<const Representation> Representation::withTrackWindow(std::int32_t c,std::int32_t h,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,bool ending,std::stop_token token)const{std::vector<std::uint8_t> bytes(static_cast<std::size_t>(trackByteLength()));readTrackBytes(c,h,0,bytes,token);return withTrackBytes(c,h,writeSignalWindow(bytes,sampleRate(),startNanoseconds(),period,start,duration,offsets,ending,token));}
}
