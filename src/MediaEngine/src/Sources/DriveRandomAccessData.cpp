#include "DriveRandomAccessData.h"
#include "DriveAccess.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
DriveRandomAccessData::DriveRandomAccessData(std::shared_ptr<const M::RandomAccessData> source,std::shared_ptr<DriveMechanics> drive,std::function<DrivePosition(std::int64_t)> position,std::function<std::int64_t(std::int64_t)> contiguous,bool writing):source_(std::move(source)),drive_(std::move(drive)),position_(std::move(position)),contiguous_(std::move(contiguous)),writing_(writing){}
std::uint64_t DriveRandomAccessData::length()const noexcept{return source_->length();}
void DriveRandomAccessData::readExactly(std::uint64_t offset,M::MutableBytes target,std::stop_token token)const {
    if(offset>length()||target.size()>length()-offset)throw M::MediaError(M::DiagnosticCode::BlockReadRange);
    if(offset>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw M::MediaError(M::DiagnosticCode::BlockOverflow);
    std::size_t completed=0;
    while(completed<target.size()){
        M::checkCancellation(token);
        if(completed>std::uint64_t(std::numeric_limits<std::int64_t>::max())-offset)throw M::MediaError(M::DiagnosticCode::BlockOverflow);
        const auto address=std::int64_t(offset+completed),contiguous=contiguous_(address);
        if(contiguous<=0)throw M::MediaError(M::DiagnosticCode::BlockDriveCount);
        const auto count=std::min(target.size()-completed,std::size_t(contiguous));
        if(count-1>std::uint64_t(std::numeric_limits<std::int64_t>::max()-address))throw M::MediaError(M::DiagnosticCode::BlockOverflow);
        DriveAccessScope access(drive_.get(),position_(address),writing_);
        source_->readExactly(std::uint64_t(address),target.subspan(completed,count),token);drive_->advance(position_(address+std::int64_t(count)-1));access.finish();completed+=count;
    }
}
}
