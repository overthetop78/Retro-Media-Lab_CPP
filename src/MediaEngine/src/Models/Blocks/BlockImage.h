#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include <RetroMediaLab/MediaEngine/RepresentationInfo.h>
#include <RetroMediaLab/MediaFileSystems/Representations.h>
#include <functional>
#include <optional>
#include <utility>
namespace RetroMediaLab::MediaEngine {
namespace Blocks {struct State;}
enum class DataRangeKind {Stored,Zero,Unallocated,Unavailable};
class HardDiskGeometry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API HardDiskGeometry(std::int64_t cylinders,std::int32_t heads,std::int32_t sectorsPerTrack,std::int32_t sectorSize);
    std::int64_t cylinders()const noexcept{return cylinders_;}std::int32_t heads()const noexcept{return heads_;}std::int32_t sectorsPerTrack()const noexcept{return sectors_;}std::int32_t sectorSize()const noexcept{return size_;}
    std::int64_t logicalSectorCount()const noexcept{return count_;}std::int64_t capacity()const noexcept{return capacity_;}
private:
    std::int64_t cylinders_,count_,capacity_;std::int32_t heads_,sectors_,size_;
};
class DataRange final {
public:
    RETROMEDIALAB_MEDIAENGINE_API DataRange(std::int64_t address,std::int64_t length,DataRangeKind,std::shared_ptr<const MediaFileSystems::RandomAccessData> source={},std::int64_t sourceOffset=0);
    std::int64_t address()const noexcept{return address_;}std::int64_t length()const noexcept{return length_;}DataRangeKind kind()const noexcept{return kind_;}
    const std::shared_ptr<const MediaFileSystems::RandomAccessData>& source()const noexcept{return source_;}std::int64_t sourceOffset()const noexcept{return sourceOffset_;}
private:
    std::int64_t address_,length_;DataRangeKind kind_;std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;std::int64_t sourceOffset_;
};
class BlockImage;
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const BlockImage> bindBlocks(std::shared_ptr<const BlockImage>,std::shared_ptr<DriveMechanics>,bool writing=false);
class BlockImage final : public MediaFileSystems::BlockRepresentation,public RepresentationInfo {
public:
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::Blocks;}
    RETROMEDIALAB_MEDIAENGINE_API BlockImage(std::int64_t logicalLength,std::int32_t logicalBlockSize,std::vector<DataRange>,std::optional<HardDiskGeometry> geometry={});
    RETROMEDIALAB_MEDIAENGINE_API BlockImage(std::int64_t logicalLength,std::vector<std::pair<std::int64_t,std::int64_t>> unavailableRanges);
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::int64_t> logicalLength()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t capacity()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t logicalBlockSize()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t logicalBlockCount()const override;
    bool supportsRandomAccess()const override{return true;}bool supportsSequentialAccess()const override{return true;}
    RETROMEDIALAB_MEDIAENGINE_API std::span<const DataRange> ranges()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API const std::optional<HardDiskGeometry>& geometry()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<DriveMechanics> drive()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API bool driveWriting()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API bool hasCompleteReadableCoverage()const;
    RETROMEDIALAB_MEDIAENGINE_API void readExactly(std::int64_t,MediaFileSystems::MutableBytes,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> readExactlyAsync(std::int64_t,MediaFileSystems::MutableBytes,std::stop_token={})const override;
private:
    explicit BlockImage(std::shared_ptr<const Blocks::State> state):state_(std::move(state)){}
    friend RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const BlockImage> bindBlocks(std::shared_ptr<const BlockImage>,std::shared_ptr<DriveMechanics>,bool);
    std::shared_ptr<const Blocks::State> state_;
};
RETROMEDIALAB_MEDIAENGINE_API void addBlockRange(std::vector<DataRange>&,std::int64_t,std::int64_t,DataRangeKind,std::shared_ptr<const MediaFileSystems::RandomAccessData> source={},std::int64_t sourceOffset=0,bool checkedLength=false);

using BlockOutput=std::function<void(MediaFileSystems::Bytes)>;
RETROMEDIALAB_MEDIAENGINE_API void copyBlocks(const BlockImage&,const BlockOutput&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void copyStoredRange(const DataRange&,const BlockOutput&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> readStoredRange(const DataRange*,std::int64_t relativeOffset,std::int64_t length,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> readStoredRange(const DataRange*,std::stop_token={});
}
