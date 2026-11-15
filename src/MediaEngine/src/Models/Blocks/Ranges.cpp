#include "Internal.h"
#include <algorithm>
#include <bit>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace Blocks {
std::int64_t add(std::int64_t left,std::int64_t right){if((right>0&&left>std::numeric_limits<std::int64_t>::max()-right)||(right<0&&left<std::numeric_limits<std::int64_t>::min()-right))throw M::MediaError(M::DiagnosticCode::BlockOverflow);return left+right;}
std::int64_t multiply(std::int64_t left,std::int64_t right){if(left<0||right<0||(right&&left>std::numeric_limits<std::int64_t>::max()/right))throw M::MediaError(M::DiagnosticCode::BlockOverflow);return left*right;}
std::int64_t wrapAdd(std::int64_t left,std::int64_t right)noexcept{return std::bit_cast<std::int64_t>(std::uint64_t(left)+std::uint64_t(right));}
}
HardDiskGeometry::HardDiskGeometry(std::int64_t cylinders,std::int32_t heads,std::int32_t sectors,std::int32_t size):cylinders_(cylinders),count_(0),capacity_(0),heads_(heads),sectors_(sectors),size_(size){
    using namespace Blocks;
    if(cylinders<=0)throw M::MediaError(M::DiagnosticCode::BlockGeometryValue,{std::string(CylindersParameter)});
    if(heads<=0)throw M::MediaError(M::DiagnosticCode::BlockGeometryValue,{std::string(HeadsParameter)});
    if(sectors<=0)throw M::MediaError(M::DiagnosticCode::BlockGeometryValue,{std::string(SectorsParameter)});
    if(size<=0)throw M::MediaError(M::DiagnosticCode::BlockGeometryValue,{std::string(SectorSizeParameter)});
    count_=multiply(multiply(cylinders,heads),sectors);capacity_=multiply(count_,size);
}
DataRange::DataRange(std::int64_t address,std::int64_t length,DataRangeKind kind,std::shared_ptr<const M::RandomAccessData> source,std::int64_t sourceOffset):address_(address),length_(length),kind_(kind),source_(std::move(source)),sourceOffset_(sourceOffset){
    if(address<0)throw M::MediaError(M::DiagnosticCode::BlockRangeAddress);
    if(length<=0)throw M::MediaError(M::DiagnosticCode::BlockRangeLength);
    if(sourceOffset<0)throw M::MediaError(M::DiagnosticCode::BlockSourceOffset);
    if(kind==DataRangeKind::Stored){if(!source_)throw M::MediaError(M::DiagnosticCode::BlockSourceNull);if(std::uint64_t(sourceOffset)>source_->length()||std::uint64_t(length)>source_->length()-std::uint64_t(sourceOffset))throw M::MediaError(M::DiagnosticCode::BlockSourceRange);}
    else {if(source_)throw M::MediaError(M::DiagnosticCode::BlockNonStoredSource);if(sourceOffset)throw M::MediaError(M::DiagnosticCode::BlockNonStoredOffset);}
}
BlockImage::BlockImage(std::int64_t length,std::int32_t size,std::vector<DataRange> ranges,std::optional<HardDiskGeometry> geometry){
    if(length<0)throw M::MediaError(M::DiagnosticCode::BlockLogicalLength);if(size<=0)throw M::MediaError(M::DiagnosticCode::BlockLogicalBlockSize);
    if(length%size)throw M::MediaError(M::DiagnosticCode::BlockCompleteBlocks);if(geometry&&geometry->capacity()>length)throw M::MediaError(M::DiagnosticCode::BlockGeometryOutside);
    std::stable_sort(ranges.begin(),ranges.end(),[](const auto& left,const auto& right){return left.address()<right.address();});std::int64_t previous=0;
    for(const auto& range:ranges){const auto end=Blocks::add(range.address(),range.length());if(end>length)throw M::MediaError(M::DiagnosticCode::BlockRangeOutside);if(range.address()<previous)throw M::MediaError(M::DiagnosticCode::BlockRangeOverlap);previous=end;}
    state_=std::make_shared<Blocks::State>(Blocks::State{length,size,std::move(ranges),std::move(geometry),{},false});
}
BlockImage::BlockImage(std::int64_t length,std::vector<std::pair<std::int64_t,std::int64_t>> ranges):BlockImage(length,1,[&ranges]{std::vector<DataRange> values;values.reserve(ranges.size());for(const auto& [address,count]:ranges)values.emplace_back(address,count,DataRangeKind::Unavailable);return values;}()){}
std::optional<std::int64_t> BlockImage::logicalLength()const{return state_->capacity;}
std::int64_t BlockImage::capacity()const{return state_->capacity;}
std::int32_t BlockImage::logicalBlockSize()const{return state_->blockSize;}
std::int64_t BlockImage::logicalBlockCount()const{return state_->capacity/state_->blockSize;}
std::span<const DataRange> BlockImage::ranges()const noexcept{return state_->ranges;}
const std::optional<HardDiskGeometry>& BlockImage::geometry()const noexcept{return state_->geometry;}
std::shared_ptr<DriveMechanics> BlockImage::drive()const noexcept{return state_->drive;}
bool BlockImage::driveWriting()const noexcept{return state_->writing;}
bool BlockImage::hasCompleteReadableCoverage()const{std::int64_t cursor=0;for(const auto& range:state_->ranges){if(range.address()!=cursor||range.kind()==DataRangeKind::Unavailable)return false;cursor=Blocks::add(cursor,range.length());}return cursor==capacity();}
void addBlockRange(std::vector<DataRange>& ranges,std::int64_t address,std::int64_t length,DataRangeKind kind,std::shared_ptr<const M::RandomAccessData> source,std::int64_t sourceOffset,bool checkedLength){
    if(!ranges.empty()){const auto& previous=ranges.back();const bool continuous=kind!=DataRangeKind::Stored||(previous.source()==source&&Blocks::wrapAdd(previous.sourceOffset(),previous.length())==sourceOffset);
        if(previous.kind()==kind&&Blocks::wrapAdd(previous.address(),previous.length())==address&&continuous){ranges.back()=DataRange(previous.address(),checkedLength?Blocks::add(previous.length(),length):Blocks::wrapAdd(previous.length(),length),kind,previous.source(),previous.sourceOffset());return;}}
    ranges.emplace_back(address,length,kind,std::move(source),sourceOffset);
}
}
