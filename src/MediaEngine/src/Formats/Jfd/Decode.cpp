#include "Jfd.h"
#include "Binary.h"
#include <algorithm>
#include <map>
#include <set>
namespace RetroMediaLab::MediaEngine::Jfd {
namespace {
using namespace Constants;
using Tables=std::map<std::uint32_t,std::vector<Record>>;
Tables readTables(std::span<const std::uint8_t> bytes,std::size_t tracks,std::size_t sectors,std::size_t data,std::uint32_t count,std::stop_token token) {
    Tables result;
    for(std::uint32_t track=0;track<count;++track){checkCancellation(token);const auto relative=Binary::read(bytes,tracks+track*FieldBytes);if(relative==Absent)continue;
        if(relative>data-sectors||relative%FieldBytes!=0)throw MediaError(DiagnosticCode::JfdTrackOffset);
        auto position=sectors+relative;auto& records=result[track];
        for(;;){Binary::require(bytes,position,FieldBytes,data);const auto header=Binary::read(bytes,position);if(header==Absent)break;
            Binary::require(bytes,position,DescriptorBytes,data);const auto location=Binary::read(bytes,position+FieldBytes);
            const auto size=header&SizeMask,density=(header>>DensityShift)&SizeMask;
            if(size>MaximumSizeCode||std::find(Densities.begin(),Densities.end(),density)==Densities.end())throw MediaError(DiagnosticCode::JfdSectorEncoding);
            std::optional<std::size_t> offset;std::size_t length=std::size_t(BaseSectorBytes)<<size;
            if(location!=Absent){if(location>bytes.size()-data)throw MediaError(DiagnosticCode::JfdTruncated);offset=data+location;
                if(header&RawTrack){Binary::require(bytes,*offset,FieldBytes,bytes.size());length=std::size_t(FieldBytes)+Binary::read(bytes,*offset);}
                Binary::require(bytes,*offset,length,bytes.size());
            }
            records.push_back({track,std::uint32_t(records.size()),header,offset,length,position});
            if(records.size()>MaximumRecords)throw MediaError(DiagnosticCode::JfdRecordCount);position+=DescriptorBytes;
        }
    }return result;
}
}
bool Record::rawTrack() const noexcept{return (header&Constants::RawTrack)!=0;}
Image decode(std::span<const std::uint8_t> source,std::stop_token token) {
    using namespace Constants;Image result;result.bytes=unpack(source,token);const std::span<const std::uint8_t> bytes(result.bytes);
    if(bytes.size()<MinimumHeader||!std::equal(Signature.begin(),Signature.end(),bytes.begin()))throw MediaError(DiagnosticCode::JfdHeader);
    if(Binary::read(bytes,LengthOffset)!=bytes.size())throw MediaError(DiagnosticCode::JfdDeclaredLength);
    const auto tracks=Binary::offset(bytes,TracksOffset),sectors=Binary::offset(bytes,SectorsOffset),data=Binary::offset(bytes,DataOffset);
    if(tracks<MinimumHeader||tracks>sectors||sectors>data||(sectors-tracks)%FieldBytes!=0)throw MediaError(DiagnosticCode::JfdTables);
    const auto trackCount=(sectors-tracks)/FieldBytes;if(trackCount==0||trackCount>MaximumTracks)throw MediaError(DiagnosticCode::JfdTrackCount);
    auto active=readTables(bytes,tracks,sectors,data,std::uint32_t(trackCount),token);
    const auto dt=Binary::offset(bytes,DeltaTracksOffset),ds=Binary::offset(bytes,DeltaSectorsOffset),dd=Binary::offset(bytes,DeltaDataOffset);
    if(dt!=0||ds!=0||dd!=0){if(dt<data||ds<dt||dd<ds||ds-dt!=trackCount*FieldBytes)throw MediaError(DiagnosticCode::JfdDeltaTables);
        auto changes=readTables(bytes,dt,ds,dd,std::uint32_t(trackCount),token);for(auto& [track,records]:changes)active[track]=std::move(records);
    }
    const auto cylinders=std::uint32_t(std::max<std::size_t>(1,(trackCount+1)/Heads));std::uint32_t heads=1,spt=1;
    std::vector<std::pair<std::size_t,std::size_t>> sizes;bool zeroBased=false,nativeLayout=false;
    for(auto& [track,records]:active){if(track%Heads!=0)heads=Heads;std::set<std::uint32_t> numbers;
        for(auto& record:records){result.records.push_back(record);nativeLayout|=record.rawTrack()||!record.offset||(record.header>>PositionShift)!=ByteMask;
            if(record.rawTrack()){result.hasRawTracks=true;continue;}
            const auto number=(record.header>>NumberShift)&ByteMask;numbers.insert(number);zeroBased|=number==0;result.hasRotationalPositions|=(record.header>>PositionShift)!=ByteMask;
            auto found=std::find_if(sizes.begin(),sizes.end(),[&](const auto& size){return size.first==record.length;});if(found==sizes.end())sizes.emplace_back(record.length,1);else ++found->second;
        }spt=std::max(spt,std::uint32_t(numbers.size()));
    }
    const auto best=std::max_element(sizes.begin(),sizes.end(),[](const auto& a,const auto& b){return a.second<b.second;});
    const auto size=best==sizes.end()?DefaultSectorBytes:std::uint32_t(best->first);
    const auto nominal=std::uint64_t(cylinders)*heads*spt;auto extra=nominal;std::set<std::uint64_t> used;std::vector<MediaFileSystems::SectorBlock> blocks;
    for(const auto& record:result.records){checkCancellation(token);if(record.rawTrack())continue;
        const auto number=(record.header>>NumberShift)&ByteMask,logicalTrack=record.track/Heads*heads+record.track%Heads;
        auto logical=std::int64_t(logicalTrack)*spt+number-(zeroBased?0:1);
        if(logical<std::int64_t(logicalTrack)*spt||logical>=std::int64_t(logicalTrack+1)*spt||!used.insert(std::uint64_t(logical)).second){logical=std::int64_t(extra++);used.insert(std::uint64_t(logical));}
        if(!record.offset)continue;
        MediaFileSystems::SectorBlock block;block.logicalBlock=std::uint64_t(logical);block.address={record.track/Heads,record.track%Heads,number};
        block.data.assign(bytes.begin()+*record.offset,bytes.begin()+*record.offset+record.length);
        const bool idValid=(record.header&HeaderError)==0,dataValid=(record.header&DataError)==0;block.integrityValid=idValid&&dataValid;block.headerIntegrityValid=idValid;block.dataIntegrityValid=dataValid;
        block.tag=std::vector<std::uint8_t>(FieldBytes+(nativeLayout?1:0));Binary::put(*block.tag,0,record.header);if(nativeLayout)block.tag->back()=NativeTag;
        block.formatCode=std::uint8_t((record.header>>DensityShift)&SizeMask);block.recordedSizeCode=std::uint8_t(record.header&SizeMask);block.recordedTrackOrder=std::int32_t(record.order);blocks.push_back(std::move(block));
    }
    MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=true;options.logicalBlockCount=std::max(nominal,extra);
    result.sectors=sparseSectorImage({std::string(FormatId),size,cylinders,heads,spt},std::move(blocks),options);return result;
}
}
