#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace Blocks {
namespace {
std::size_t containing(const std::vector<DataRange>& ranges,std::int64_t address){std::size_t low=0,high=ranges.size();while(low<high){const auto middle=low+(high-low)/SearchDivisor;const auto& range=ranges[middle];if(address<range.address())high=middle;else if(address>=add(range.address(),range.length()))low=middle+1;else return middle;}return low;}
std::string kindName(DataRangeKind kind){switch(kind){case DataRangeKind::Stored:return std::string(StoredName);case DataRangeKind::Zero:return std::string(ZeroName);case DataRangeKind::Unallocated:return std::string(UnallocatedName);case DataRangeKind::Unavailable:return std::string(UnavailableName);}return std::to_string(int(kind));}
}
void read(const State& state,std::int64_t address,M::MutableBytes destination,std::stop_token token){
    if(address<0)throw M::MediaError(M::DiagnosticCode::BlockReadAddress);
    if(address>state.capacity||destination.size()>std::uint64_t(state.capacity-address))throw M::MediaError(M::DiagnosticCode::BlockReadRange);
    if(destination.empty())return;
    auto index=containing(state.ranges,address);std::size_t completed=0;
    while(completed<destination.size()){
        M::checkCancellation(token);if(index>=state.ranges.size())throw M::MediaError(M::DiagnosticCode::BlockRangeMissing,{std::to_string(address)});
        const auto& range=state.ranges[index];if(address<range.address()||address>=add(range.address(),range.length()))throw M::MediaError(M::DiagnosticCode::BlockRangeMissing,{std::to_string(address)});
        const auto relative=address-range.address();auto count=std::min(destination.size()-completed,std::size_t(range.length()-relative));
        if(state.drive){const auto contiguous=contiguousLength(state,address);if(contiguous<=0)throw M::MediaError(M::DiagnosticCode::BlockDriveCount);count=std::min(count,std::size_t(contiguous));}
        const auto target=destination.subspan(completed,count);const bool direct=state.drive&&(range.kind()==DataRangeKind::Zero||range.kind()==DataRangeKind::Unallocated);
        Access access(direct?state.drive.get():nullptr,direct?position(state,address):DrivePosition{},state.writing);
        switch(range.kind()){
        case DataRangeKind::Stored:range.source()->readExactly(std::uint64_t(add(range.sourceOffset(),relative)),target,token);break;
        case DataRangeKind::Zero:case DataRangeKind::Unallocated:std::fill(target.begin(),target.end(),std::uint8_t{});break;
        case DataRangeKind::Unavailable:throw M::MediaError(M::DiagnosticCode::BlockUnavailable,{std::to_string(address)});
        default:throw M::MediaError(M::DiagnosticCode::BlockRangeKind,{kindName(range.kind())});
        }
        if(direct)state.drive->advance(position(state,add(address,std::int64_t(count)-1)));access.finish();
        address+=std::int64_t(count);completed+=count;if(relative+std::int64_t(count)==range.length())++index;
    }
}
}
void BlockImage::readExactly(std::int64_t address,M::MutableBytes target,std::stop_token token)const{Blocks::read(*state_,address,target,token);}
std::future<void> BlockImage::readExactlyAsync(std::int64_t address,M::MutableBytes target,std::stop_token token)const{return std::async(std::launch::deferred,[state=state_,address,target,token]{Blocks::read(*state,address,target,token);});}
}
