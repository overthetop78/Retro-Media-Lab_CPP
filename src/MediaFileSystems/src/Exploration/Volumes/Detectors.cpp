#include <RetroMediaLab/MediaFileSystems/VolumeDetectors.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include "../../Partitioning/PartitionConstants.h"
#include <filesystem>
#include <limits>

namespace RetroMediaLab::MediaFileSystems {
namespace {
enum class DetectorKind {Sequential,Optical,Gpt,Mbr,Whole};
std::int64_t product(std::int64_t left,std::int32_t right) {
    constexpr auto high=std::numeric_limits<std::int64_t>::max(),low=std::numeric_limits<std::int64_t>::min();
    if((left>0&&((right>0&&left>high/right)||(right<0&&right<low/left)))||
       (left<0&&((right>0&&left<low/right)||(right<0&&left<high/right))))throw MediaError(DiagnosticCode::VolumeRangeOverflow);
    return left*right;
}
std::optional<std::int64_t> contentLength(const ImageDocument& document) {
    if(const auto value=document.representation().logicalLength())return value;
    if(const auto value=document.source().knownLength())return value;
    const auto& name=document.source().primaryPath();
    const std::filesystem::path path(std::u8string(reinterpret_cast<const char8_t*>(name.data()),name.size()));
    std::error_code error;if(!std::filesystem::is_regular_file(path,error))return {};
    const auto length=std::filesystem::file_size(path,error);
    if(error)throw MediaError(DiagnosticCode::SourceLengthFailed,{name});
    if(length>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::VolumeRangeOverflow);
    return std::int64_t(length);
}
class BlockSource final:public RandomAccessData {
    std::shared_ptr<const ImageDocument> owner_;
    const BlockRepresentation& blocks_;
    std::uint64_t length_;
public:
    BlockSource(std::shared_ptr<const ImageDocument> owner,const BlockRepresentation& blocks):owner_(std::move(owner)),blocks_(blocks),length_(std::uint64_t(blocks.capacity())){}
    std::uint64_t length()const noexcept override{return length_;}
    void readExactly(std::uint64_t offset,MutableBytes destination,std::stop_token token)const override {
        checkCancellation(token);
        if(offset>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::VolumeRangeOverflow);
        blocks_.readExactlyAsync(std::int64_t(offset),destination,token).get();
    }
};
std::optional<VolumeDetectionResult> detect(std::shared_ptr<const ImageDocument> document,DetectorKind kind,std::stop_token token) {
    if(!document)throw MediaError(DiagnosticCode::DocumentNull);checkCancellation(token);
    const auto& representation=document->representation();
    if(kind==DetectorKind::Sequential||kind==DetectorKind::Whole) {
        const auto sequential=kind==DetectorKind::Sequential;
        if(sequential&&(!document->isTape()||representation.supportsRandomAccess()||!representation.supportsSequentialAccess()))return {};
        const auto length=contentLength(*document);
        if(!length||*length<=0)return VolumeDetectionResult{{},{{sequential?DiagnosticCode::SequentialNoLength:DiagnosticCode::NoVolumeLength,{}}}};
        return VolumeDetectionResult{{VolumeDescriptor{0,*length,std::string(sequential?VolumeConstants::SequentialContent:VolumeConstants::DirectVolume),{},{},{},{},sequential?std::optional<std::string>(FileSystemIds::SequentialContent):std::nullopt}},{}};
    }
    if(kind==DetectorKind::Optical) {
        const auto* optical=dynamic_cast<const OpticalRepresentation*>(&representation);if(!optical)return {};
        VolumeDetectionResult result;
        if(const auto tracks=optical->tracks())for(const auto& track:*tracks) {
            if(!track)throw MediaError(DiagnosticCode::InvalidSource);if(track->isAudio())continue;
            result.volumes.emplace_back(product(track->firstSector(),track->userDataLength()),product(track->sectorCount(),track->userDataLength()),std::string(VolumeConstants::OpticalTrack),std::nullopt,std::nullopt,track->sessionNumber(),track->trackNumber());
        }
        if(result.volumes.empty())result.diagnostics.push_back({DiagnosticCode::OpticalNoDataTrack,{}});
        return result;
    }
    const auto* blocks=dynamic_cast<const BlockRepresentation*>(&representation);if(!blocks||!document->isHardDisk())return {};
    const auto size=blocks->logicalBlockSize();const auto capacity=blocks->capacity(),count=blocks->logicalBlockCount();
    if(kind==DetectorKind::Mbr) {
        if(size<std::int64_t(PartitionConstants::BootRecordSize)||capacity<size)return {};
        if(count<=0)throw MediaError(DiagnosticCode::MbrBootOutside);
    }else if(count<std::int64_t(PartitionConstants::MinimumGptBlockCount))return {};
    if(size<0)throw MediaError(DiagnosticCode::InvalidBlockSize,{},ErrorKind::Overflow);
    if(size==0)return {};
    if(capacity<0)throw MediaError(DiagnosticCode::InvalidSource);
    auto source=std::make_shared<BlockSource>(document,*blocks);
    const BlockMedia media(std::move(source),std::uint32_t(size),true,std::uint64_t(count));
    return kind==DetectorKind::Mbr?detectMbr(media,token):detectGpt(media,token);
}
class Detector final:public VolumeDetector {
    DetectorKind kind_;
public:
    explicit Detector(DetectorKind kind):kind_(kind){}
    std::future<std::optional<VolumeDetectionResult>> detectAsync(std::shared_ptr<const ImageDocument> document,std::stop_token token)const override {
        return std::async(std::launch::deferred,[document=std::move(document),kind=kind_,token]{return detect(document,kind,token);});
    }
};
}
std::vector<std::shared_ptr<const VolumeDetector>> createDefaultVolumeDetectors() {
    return {std::make_shared<Detector>(DetectorKind::Sequential),std::make_shared<Detector>(DetectorKind::Optical),std::make_shared<Detector>(DetectorKind::Gpt),std::make_shared<Detector>(DetectorKind::Mbr),std::make_shared<Detector>(DetectorKind::Whole)};
}
}
