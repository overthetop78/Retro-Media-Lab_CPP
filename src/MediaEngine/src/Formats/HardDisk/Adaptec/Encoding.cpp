#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <array>
namespace RetroMediaLab::MediaEngine::Adaptec {
void appendByte(Flux::Bits& bits,std::uint8_t value,bool& previous){for(auto bit=Constants::BitsPerByte-1;bit>=0;--bit){const auto data=(value&(1<<bit))!=0;bits.push_back(std::uint8_t(!previous&&!data));bits.push_back(std::uint8_t(data));previous=data;}}
void appendField(Flux::Bits& bits,std::span<const std::uint8_t> record,bool& previous){for(auto bit=Constants::CellsPerByte-1;bit>=0;--bit)bits.push_back(std::uint8_t((Constants::SyncWord&(1<<bit))!=0));previous=true;for(auto value:record.subspan(1))appendByte(bits,value,previous);}
namespace {
void gap(Flux::Bits& bits,std::uint8_t value,std::int32_t count,bool& previous){for(auto i=0;i<count;++i)appendByte(bits,value,previous);}
}
Flux::NanosecondRevolution encode(std::span<const MediaFileSystems::SectorBlock> sectors,bool sequential,std::uint32_t period,std::int32_t hint,std::span<const std::uint8_t> parameters,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(sectors.empty()||sectors.size()>MaximumSectors)throw MediaError(DiagnosticCode::AdaptecSectors);if(hint&&!validSize(std::size_t(hint)))throw MediaError(DiagnosticCode::AdaptecSectorSize);
    if(!parameters.empty()&&(parameters.size()!=ParameterFlagCount||sectors.size()<ParameterFlagCount||std::any_of(parameters.begin(),parameters.end(),[](auto value){return value>DefectCountMask;})))throw MediaError(DiagnosticCode::AdaptecParameterValue);
    const auto cells=period/CellNanoseconds;if(!cells)throw MediaError(DiagnosticCode::AdaptecPeriod);Flux::Bits bits;bits.reserve(cells);bool previous=false;const auto maximum=std::max_element(sectors.begin(),sectors.end(),[](const auto& a,const auto& b){return a.address.number<b.address.number;})->address.number;
    std::array<std::uint8_t,IdLength> id{};std::vector<std::uint8_t> record;
    for(std::size_t index=0;index<sectors.size();++index){checkCancellation(token);const auto& sector=sectors[index];const auto first=index==0||!sequential;
        if(sector.data.empty()&&hint){gap(bits,GapByte,hint+(first?InterleavedRecordOverhead:SequentialRecordOverhead),previous);continue;}
        if(sector.address.number<0||std::uint64_t(sector.address.number)>AddressMask||!validSize(sector.data.size()))throw MediaError(DiagnosticCode::AdaptecSectors);
        if(sector.deletedDataMark==true||sector.recordedSizeCode||(!sector.recordedErrorCheckCode.empty()&&sector.recordedErrorCheckCode.size()!=CheckLength))throw MediaError(DiagnosticCode::AdaptecUnsupportedSector);
        gap(bits,GapByte,first?FirstGapLength:SequentialGapLength,previous);gap(bits,0,IdPreambleLength,previous);id.fill(0);id[0]=SyncByte;id[1]=IdMark;
        for(std::size_t part=0;part<AddressLength;++part)id[AddressOffset+part]=std::uint8_t(std::uint64_t(sector.address.number)>>((AddressLength-1-part)*BitsPerByte));id[FlagsOffset]=sector.address.number==maximum?LastSectorFlag:0;if(index<parameters.size())id[FlagsOffset]|=parameters[index];writeErrorCheck(id);if(sector.headerIntegrityValid==false)id.back()^=InvalidCheckMask;appendField(bits,id,previous);
        gap(bits,0,DataPreambleLength,previous);record.resize(sector.data.size()+DataOverhead);record[0]=SyncByte;record[1]=DataMark;std::copy(sector.data.begin(),sector.data.end(),record.begin()+DataOffset);writeErrorCheck(record);
        if(sector.recordedErrorCheckCode.size()==CheckLength)std::copy(sector.recordedErrorCheckCode.begin(),sector.recordedErrorCheckCode.end(),record.end()-CheckLength);
        else if(sector.dataIntegrityValid==false||sector.integrityValid==false)record.back()^=InvalidCheckMask;
        appendField(bits,record,previous);gap(bits,0,PostDataLength,previous);
    }
    if(bits.size()>cells)throw MediaError(DiagnosticCode::AdaptecTrackCapacity);while(bits.size()<cells)gap(bits,GapByte,1,previous);bits.resize(cells);auto flux=Flux::createRevolution(bits,CellNanoseconds,period,false,token);return {period,std::move(flux.intervals)};
}
}
