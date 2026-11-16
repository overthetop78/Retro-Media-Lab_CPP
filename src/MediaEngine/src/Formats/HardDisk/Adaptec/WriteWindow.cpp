#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Adaptec {
WriteWindow createSectorWrite(const Flux::NanosecondRevolution& revolution,std::int32_t cylinder,std::int32_t head,std::int64_t logical,std::span<const std::uint8_t> payload,std::stop_token token){
    using namespace Constants;if(!validSize(payload.size()))throw MediaError(DiagnosticCode::AdaptecSectorSize);Locations locations;const auto fields=decodeCore(revolution,cylinder,head,std::int32_t(payload.size()),&locations,token);const auto found=locations.find(logical);
    if(found==locations.end()||found->second.offset<0||std::count_if(fields.begin(),fields.end(),[&](const auto& field){return std::int64_t(field.sector.logicalBlock)==logical&&field.sector.headerIntegrityValid==true&&(field.id[AddressOffset]&DefectiveAddressByteMask)==0;})!=1)throw MediaError(DiagnosticCode::AdaptecWriteIdentity);
    const auto location=found->second;std::vector<std::uint8_t> record(payload.size()+DataOverhead);record[0]=SyncByte;record[1]=DataMark;std::copy(payload.begin(),payload.end(),record.begin()+DataOffset);writeErrorCheck(record);Flux::Bits bits;bits.reserve(record.size()*CellsPerByte+1);bool previous=false;appendField(bits,record,previous);bits.push_back(std::uint8_t(location.sync?location.nextClock:!previous&&!location.nextData));
    WriteWindow result{std::uint32_t((std::uint64_t(location.offset)+1)*CellNanoseconds%revolution.indexTime),std::uint32_t(bits.size()*CellNanoseconds),{}};
    for(std::size_t cell=0;cell<bits.size();++cell){checkCancellation(token);if(bits[cell])result.transitions.push_back(std::uint32_t(cell*CellNanoseconds));}return result;
}
}
