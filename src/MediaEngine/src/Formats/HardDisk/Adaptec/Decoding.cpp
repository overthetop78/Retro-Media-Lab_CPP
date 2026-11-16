#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Adaptec {
namespace {
std::uint8_t byteAt(Flux::BitView bits,std::int64_t offset){std::uint8_t value{};for(auto bit=0;bit<Constants::BitsPerByte;++bit)value=std::uint8_t((value<<1)|(bits[std::size_t((offset+bit*Constants::CellsPerBit+1)%std::int64_t(bits.size()))]?1:0));return value;}
std::vector<std::uint8_t> fieldAt(Flux::BitView bits,std::int32_t offset,std::size_t length){std::vector<std::uint8_t> bytes(length);for(std::size_t i=0;i<length;++i)bytes[i]=byteAt(bits,std::int64_t(offset)+std::int64_t(i)*Constants::CellsPerByte);return bytes;}
bool preamble(Flux::BitView bits,std::int32_t mark){for(auto cell=1;cell<=Constants::MinimumSyncBits*Constants::CellsPerBit;++cell){const auto offset=((std::int64_t(mark)-cell)%std::int64_t(bits.size())+std::int64_t(bits.size()))%std::int64_t(bits.size());if(bool(bits[std::size_t(offset)])!=(cell%Constants::CellsPerBit==0))return false;}return true;}
struct Mark {std::int32_t offset;std::uint8_t value;};
}
std::vector<Field> decodeCore(const Flux::NanosecondRevolution& revolution,std::int32_t cylinder,std::int32_t head,std::int32_t hint,Locations* locations,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(hint&&!validSize(std::size_t(hint)))throw MediaError(DiagnosticCode::AdaptecSectorSize);if(!revolution.indexTime)throw MediaError(DiagnosticCode::AdaptecPeriod);if(std::any_of(revolution.intervals.begin(),revolution.intervals.end(),[](auto value){return value==0;}))throw MediaError(DiagnosticCode::AdaptecField);
    const auto period=std::int32_t((std::uint64_t(revolution.indexTime)+CellNanoseconds-1)/CellNanoseconds);const auto source=Flux::reconstruct(revolution.intervals,CellNanoseconds,period,false,token);Flux::Bits bits(std::size_t(period),0);std::copy_n(source.bits().begin(),std::min(bits.size(),source.bits().size()),bits.begin());
    std::vector<Mark> marks;std::uint16_t shift{};
    for(std::int32_t offset=0;offset<period+CellsPerByte-1;++offset){checkCancellation(token);shift=std::uint16_t((shift<<1)|(bits[std::size_t(offset%period)]?1:0));if(offset<CellsPerByte-1||shift!=SyncWord)continue;const auto start=offset-CellsPerByte+1;if(preamble(bits,start))marks.push_back({start,byteAt(bits,start+CellsPerByte)});}
    std::vector<Field> result;
    for(std::size_t position=0;position<marks.size();++position){checkCancellation(token);const auto mark=marks[position];if(mark.value!=IdMark)continue;auto id=fieldAt(bits,mark.offset,IdLength);const auto validId=errorCheck(id)==0;std::uint32_t address{};for(std::size_t part=0;part<AddressLength;++part)address=(address<<BitsPerByte)|id[AddressOffset+part];
        auto nextId=mark.offset+period;std::int32_t dataOffset=-1;bool blocked=false;
        for(std::size_t following=1;following<=marks.size();++following){const auto candidate=marks[(position+following)%marks.size()];const auto offset=candidate.offset+(position+following>=marks.size()?period:0);if(candidate.value==IdMark){nextId=offset;break;}if(candidate.value!=DataMark&&dataOffset<0)blocked=true;if(!blocked&&dataOffset<0&&candidate.value==DataMark&&offset>=mark.offset+std::int32_t(IdLength)*CellsPerByte)dataOffset=offset;}
        std::vector<std::uint8_t> record;bool validData=false;
        if(dataOffset>=0)for(auto size:CandidateSizes){if(hint&&size!=hint)continue;if(dataOffset+(size+std::int32_t(DataOverhead))*CellsPerByte>nextId)continue;auto candidate=fieldAt(bits,dataOffset,std::size_t(size)+DataOverhead);const auto valid=errorCheck(candidate)==0;if(hint||valid){record=std::move(candidate);validData=valid;break;}}
        const auto defective=(address&DefectiveMask)!=0;const auto logical=address&AddressMask;
        if(locations&&validId&&!defective&&!record.empty()){
            const auto next=dataOffset+std::int32_t(record.size())*CellsPerByte;std::uint16_t word{};for(auto bit=0;bit<CellsPerByte;++bit)word=std::uint16_t((word<<1)|(bits[std::size_t((next+bit)%period)]?1:0));
            const Location location{dataOffset,bool(bits[std::size_t((next+1)%period)]),bool(bits[std::size_t(next%period)]),word==SyncWord};if(!locations->try_emplace(logical,location).second)(*locations)[logical]=Location{};
        }
        MediaFileSystems::SectorBlock sector;sector.logicalBlock=logical;sector.address={cylinder,head,logical};sector.integrityValid=validId&&validData&&!defective;sector.headerIntegrityValid=validId;
        if(!record.empty()){sector.data.assign(record.begin()+DataOffset,record.end()-CheckLength);sector.dataIntegrityValid=validData;sector.recordedDataPrefix.assign(record.begin(),record.begin()+DataOffset);sector.recordedErrorCheckCode.assign(record.end()-CheckLength,record.end());}
        result.push_back({std::move(id),std::move(sector)});
    }
    return result;
}
std::vector<Field> decode(const Flux::NanosecondRevolution& revolution,std::int32_t cylinder,std::int32_t head,std::int32_t hint,std::stop_token token){return decodeCore(revolution,cylinder,head,hint,nullptr,token);}
}
