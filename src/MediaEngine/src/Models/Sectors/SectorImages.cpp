#include "SectorImages.h"
#include "SectorConstants.h"
#include "../../Primitives/Identifiers.h"
#include <map>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace MediaFileSystems;
using namespace SectorConstants;
void formatId(std::string_view id) {
    if(Primitives::isBlankIdentifier(id))throw MediaError(DiagnosticCode::SectorInvalidFormatId,{std::string(id)});
}
void dimension(std::uint64_t value,std::string_view field) {
    if(value==0 || value>CapacityLimit)throw MediaError(DiagnosticCode::SectorInvalidGeometry,{std::string(field),std::to_string(value)});
}
std::uint64_t multiply(std::uint64_t a,std::uint64_t b,std::string_view field) {
    if(b!=0 && a>CapacityLimit/b)throw MediaError(DiagnosticCode::SectorInvalidGeometry,{std::string(field),std::string(Overflow)});
    return a*b;
}
class ImageBase : public SectorImage {
public:
    ImageBase(SectorGeometry geometry,SectorImageOptions options) : geometry_(std::move(geometry)),options_(options) {
        formatId(geometry_.formatId);dimension(geometry_.blockSize,BlockSizeField);dimension(geometry_.cylinders,CylindersField);dimension(geometry_.heads,HeadsField);dimension(geometry_.sectorsPerTrack,TrackSectorsField);
        count_=options.logicalBlockCount.value_or(0);
        if(!options.logicalBlockCount)count_=multiply(multiply(geometry_.cylinders,geometry_.heads,GeometryExpression),geometry_.sectorsPerTrack,GeometryExpression);
        dimension(count_,CountField);
        capacity_=options.capacity.value_or(0);if(!options.capacity)capacity_=multiply(count_,geometry_.blockSize,CapacityField);dimension(capacity_,CapacityField);
    }
    const SectorGeometry& geometry() const noexcept override{return geometry_;}
    std::uint64_t blockCount() const noexcept override{return count_;}
    std::uint64_t capacity() const noexcept override{return capacity_;}
    bool allowsVariableBlockSize() const noexcept override{return options_.allowVariableBlockSize;}
protected:
    SectorGeometry geometry_;SectorImageOptions options_;std::uint64_t count_=0,capacity_=0;
};
class SparseImage final : public ImageBase {
public:
    SparseImage(SectorGeometry geometry,std::vector<SectorBlock> blocks,SectorImageOptions options) : ImageBase(std::move(geometry),options) {
        std::uint64_t described=0;
        for(auto& block:blocks) {
            const auto logical=block.logicalBlock;
            if(!blocks_.emplace(logical,std::make_shared<const SectorBlock>(std::move(block))).second)throw MediaError(DiagnosticCode::SectorDuplicateBlock,{std::to_string(logical)});
            order_.push_back(logical);
        }
        for(const auto logical:order_) {
            const auto& block=blocks_.at(logical);
            if(logical>=count_)throw MediaError(DiagnosticCode::SectorBlockOutside,{std::to_string(logical),std::to_string(count_)});
            if(block->data.size()>CapacityLimit-described)throw MediaError(DiagnosticCode::SectorInvalidGeometry,{std::string(CapacityField),std::string(Overflow)});
            described+=block->data.size();
        }
        if(options.capacity && *options.capacity<described)throw MediaError(DiagnosticCode::SectorCapacityShort,{std::to_string(*options.capacity),std::to_string(described)});
    }
    bool containsBlock(std::uint64_t logical) const noexcept override{return blocks_.contains(logical);}
    std::optional<SectorAddress> blockAddress(std::uint64_t logical) const override{const auto found=blocks_.find(logical);return found==blocks_.end()?std::nullopt:std::optional(found->second->address);}
    std::shared_ptr<const SectorBlock> block(std::uint64_t logical,std::stop_token token) const override {
        checkCancellation(token);const auto found=blocks_.find(logical);return found==blocks_.end()?nullptr:found->second;
    }
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visitor,std::stop_token token) const override {
        if(!visitor)return;for(const auto logical:order_){checkCancellation(token);if(!visitor(logical))break;}
    }
private:
    std::map<std::uint64_t,std::shared_ptr<const SectorBlock>> blocks_;
    std::vector<std::uint64_t> order_;
};
class SourceImage final : public ImageBase {
public:
    SourceImage(std::shared_ptr<const RandomAccessData> source,SectorGeometry geometry,std::uint64_t offset,SectorImageOptions options,SourceSectorMetadata metadata)
        : ImageBase(std::move(geometry),options),source_(std::move(source)),offset_(offset),metadata_(metadata) {
        if(geometry_.blockSize>VolumeConstants::MaximumBlockSize)throw MediaError(DiagnosticCode::InvalidBlockSize);
        if(metadata_.firstSectorNumber<0 || metadata_.firstSectorNumber>1)throw MediaError(DiagnosticCode::SectorInvalidGeometry,{std::string(SectorNumberField),std::to_string(metadata_.firstSectorNumber)});
        if(!source_)throw MediaError(DiagnosticCode::InvalidSource);
        if(offset_>source_->length())throw MediaError(DiagnosticCode::ReadOutOfBounds);
        available_=source_->length()-offset_;
    }
    bool containsBlock(std::uint64_t logical) const noexcept override {
        return logical<count_ && logical<available_/geometry_.blockSize+(available_%geometry_.blockSize!=0);
    }
    std::optional<SectorAddress> blockAddress(std::uint64_t logical) const override {
        if(!containsBlock(logical))return {};
        const auto track=logical/geometry_.sectorsPerTrack;
        if(metadata_.addressOf)return metadata_.addressOf(logical);
        return SectorAddress{std::int64_t(track/geometry_.heads),std::int64_t(track%geometry_.heads),std::int64_t(logical%geometry_.sectorsPerTrack)+metadata_.firstSectorNumber};
    }
    std::shared_ptr<const SectorBlock> block(std::uint64_t logical,std::stop_token token) const override {
        checkCancellation(token);const auto address=blockAddress(logical);if(!address)return nullptr;
        auto result=std::make_shared<SectorBlock>();result->logicalBlock=logical;result->address=*address;
        if(metadata_.recordedHead)result->recordedAddress=SectorAddress{result->address.cylinder,*metadata_.recordedHead,result->address.number};
        const auto start=logical*geometry_.blockSize;const auto length=std::min(std::uint64_t(geometry_.blockSize),available_-start);
        result->data.resize(std::size_t(length));result->integrityValid=metadata_.trustCompletePayload && length==geometry_.blockSize?std::optional<bool>(true):std::nullopt;source_->readExactly(offset_+start,result->data,token);return result;
    }
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visitor,std::stop_token token) const override {
        if(!visitor)return;const auto count=std::min(count_,available_/geometry_.blockSize+(available_%geometry_.blockSize!=0));
        for(std::uint64_t logical=0;logical<count;++logical){checkCancellation(token);if(!visitor(logical))break;}
    }
private:
    std::shared_ptr<const RandomAccessData> source_;std::uint64_t offset_,available_=0;SourceSectorMetadata metadata_;
};
class RelabeledImage final : public SectorImage {
public:
    RelabeledImage(std::shared_ptr<const SectorImage> source,std::string id):source_(std::move(source)) {
        if(!source_)throw MediaError(DiagnosticCode::InvalidSource);formatId(id);geometry_=source_->geometry();geometry_.formatId=std::move(id);
    }
    const SectorGeometry& geometry() const noexcept override{return geometry_;}
    std::uint64_t blockCount() const noexcept override{return source_->blockCount();}
    std::uint64_t capacity() const noexcept override{return source_->capacity();}
    bool allowsVariableBlockSize() const noexcept override{return source_->allowsVariableBlockSize();}
    bool containsBlock(std::uint64_t logical) const noexcept override{return source_->containsBlock(logical);}
    std::optional<SectorAddress> blockAddress(std::uint64_t logical) const override{return source_->blockAddress(logical);}
    std::shared_ptr<const SectorBlock> block(std::uint64_t logical,std::stop_token token) const override{return source_->block(logical,token);}
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visitor,std::stop_token token) const override{source_->visitAvailableBlocks(visitor,token);}
private:
    std::shared_ptr<const SectorImage> source_;SectorGeometry geometry_;
};
}
std::shared_ptr<const MediaFileSystems::SectorImage> sparseSectorImage(MediaFileSystems::SectorGeometry geometry,std::vector<MediaFileSystems::SectorBlock> blocks,MediaFileSystems::SectorImageOptions options) {return std::make_shared<SparseImage>(std::move(geometry),std::move(blocks),options);}
std::shared_ptr<const MediaFileSystems::SectorImage> sourceSectorImage(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,MediaFileSystems::SectorGeometry geometry,std::uint64_t offset,MediaFileSystems::SectorImageOptions options,SourceSectorMetadata metadata) {return std::make_shared<SourceImage>(std::move(source),std::move(geometry),offset,options,metadata);}
std::shared_ptr<const MediaFileSystems::SectorImage> relabelSectorImage(std::shared_ptr<const MediaFileSystems::SectorImage> source,std::string formatId) {return std::make_shared<RelabeledImage>(std::move(source),std::move(formatId));}
std::shared_ptr<const MediaFileSystems::SectorImage> imageFromWritePlan(const MediaFileSystems::SectorWritePlan& plan) {
    std::vector<MediaFileSystems::SectorBlock> blocks;blocks.reserve(plan.blocks.size());
    for(const auto& block:plan.blocks){MediaFileSystems::SectorBlock value;value.logicalBlock=block.logicalBlock;value.address=block.address;value.data=block.data;blocks.push_back(std::move(value));}
    return sparseSectorImage(plan.geometry,std::move(blocks),{false,plan.capacity,plan.logicalBlockCount});
}
}
