#include "WriterInternal.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
using namespace Constants;
Builder::Builder(const MigrationPlan& plan,Geometry geometry,const WritePolicy& policy,std::stop_token token):plan_(plan),geometry_(std::move(geometry)),policy_(policy),token_(token),sectors_(geometry_.count,std::vector<std::uint8_t>(SectorSize)),free_(geometry_.count,true){}
void Builder::validate() const{
    if(!validName(plan_.volumeName))throw MediaError(DiagnosticCode::CbmWriteEntry,{std::string(RootPath)});
    for(const auto& entry:plan_.entries){
        checkCancellation(token_);
        if(entry.kind!=EntryKind::File||!entry.content||!entry.children.empty()||!validName(entry.targetName))throw MediaError(DiagnosticCode::CbmWriteEntry,{entry.sourcePath});
    }
}
SectorWritePlan Builder::build(){
    checkCancellation(token_);validate();reserveSystem();allocateDirectories();writeFiles();system();
    SectorWritePlan result;result.geometry=geometry_.geometry;
    result.capacity=std::uint64_t(geometry_.count)*SectorSize;result.logicalBlockCount=geometry_.count;result.blocks.reserve(geometry_.count);
    for(std::int32_t block=0;block<geometry_.count;++block){checkCancellation(token_);result.blocks.push_back({std::uint64_t(block),geometry_.physical(block),std::move(sectors_[block])});}
    return result;
}
}
namespace RetroMediaLab::MediaFileSystems::CommodoreDos {
SectorWritePlan write(const MigrationPlan& plan,std::string_view format,const WritePolicy& policy,std::stop_token token){
    checkCancellation(token);return Writing::Builder(plan,Writing::resolve(format),policy,token).build();
}
}
