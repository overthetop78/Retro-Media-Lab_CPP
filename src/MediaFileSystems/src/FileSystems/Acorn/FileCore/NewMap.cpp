#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include "Constants.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::FileCore {
namespace {
using namespace Constants;
struct Zone {std::vector<std::uint8_t> data;std::int32_t startMapBit,startBit,endBit;};
std::int32_t narrow(std::int64_t value){
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::FileCoreOverflow);
    return std::int32_t(value);
}
std::int32_t mapAddress(const DiscRecord& record){
    const auto before=std::int64_t(record.zoneCount>>1)*record.zoneSizeBits();
    const auto correction=record.zoneCount>1?RecordLength*BitsPerByte:0;
    return applyShift(narrow(before-correction),record.log2BytesPerMapBit-record.log2SectorSize);
}
std::uint8_t zoneCheck(Bytes data){
    std::uint32_t a=0,b=0,c=0,d=0;
    for(auto offset=std::int64_t(data.size())-WordBytes;offset>0;offset-=WordBytes){
        a+=data[std::size_t(offset)]+(d>>BitsPerByte);d&=ByteMask;
        b+=data[std::size_t(offset+1)]+(a>>BitsPerByte);a&=ByteMask;
        c+=data[std::size_t(offset+PairCount)]+(b>>BitsPerByte);b&=ByteMask;
        d+=data[std::size_t(offset+UInt24Size)]+(c>>BitsPerByte);c&=ByteMask;
    }
    a+=d>>BitsPerByte;b+=data[1]+(a>>BitsPerByte);c+=data[ZoneTerminatorOffset]+(b>>BitsPerByte);d+=data[ZoneCrossCheckOffset]+(c>>BitsPerByte);
    return std::uint8_t(a^b^c^d);
}
std::optional<DiscRecord> mapRecord(const SectorImage& image,std::uint64_t address,std::stop_token token){
    checkCancellation(token);if(!image.containsBlock(address))return {};
    const auto block=image.block(address,token);
    if(!block||block->data.size()<RecordOffset+RecordLength)return {};
    return parseDiscRecord(Bytes(block->data).subspan(RecordOffset,RecordLength),image.capacity());
}
std::optional<DiscRecord> bootRecord(const SectorImage& image,std::stop_token token){
    const auto size=image.geometry().blockSize;if(size==0)return {};
    const auto address=BootAddress/size,offset=BootAddress%size;
    if(!image.containsBlock(address))return {};
    const auto block=image.block(address,token);
    if(!block||offset>block->data.size()||BootLength>block->data.size()-offset)return {};
    const auto boot=Bytes(block->data).subspan(offset,BootLength);
    if(!validChecksum(boot))return {};
    return parseDiscRecord(boot.subspan(BootRecordOffset,RecordLength),image.capacity());
}
std::optional<Zone> createZone(std::vector<std::uint8_t> data,int index,const DiscRecord& record,std::int32_t described){
    const auto start=ZoneHeaderBits+(index==0?RecordLength*BitsPerByte:0);
    const auto startMap=index==0?0:std::int64_t(index)*record.zoneSizeBits()-RecordLength*BitsPerByte;
    const auto preceding=std::int64_t(record.zoneCount-1)*record.zoneSizeBits()-RecordLength*BitsPerByte;
    const auto length=index==record.zoneCount-1?described-preceding:record.zoneSizeBits();
    const auto end=ZoneHeaderBits+length;
    if(start>=end||end>std::int64_t(data.size())*BitsPerByte||startMap<0)return {};
    return Zone{std::move(data),narrow(startMap),start,narrow(end)};
}
class NewMap final:public AddressResolver {
    DiscRecord record_;std::vector<Zone> zones_;std::uint64_t capacity_,blockCount_,free_=0;std::uint32_t blockSize_;
    std::int32_t shift_,idsPerZone_;
    bool lookup(const Zone& zone,std::uint32_t id,std::int32_t& remaining,std::int32_t& found,std::stop_token token) const{
        const auto mask=(std::uint32_t(1)<<record_.idLength)-1;
        const auto first=getBits(zone.data,FreeLinkBit,mask&FreeMask);
        auto freeLink=first==0?0:narrow(FreeLinkBit+std::int64_t(first));
        auto start=zone.startBit;
        while(start<zone.endBit){
            checkCancellation(token);
            const auto fragment=getBits(zone.data,start,mask);
            const auto end=findNextSetBit(zone.data,start+record_.idLength,zone.endBit);
            if(end>=zone.endBit)return false;
            if(start==freeLink)freeLink=narrow(std::int64_t(freeLink)+(fragment&FreeMask));
            else if(fragment==id){const auto length=end+1-start;if(remaining>=length)remaining-=std::int32_t(length);else {found=start+remaining;return true;}}
            start=std::int32_t(end+1);
        }
        return false;
    }
    std::uint64_t countFree(std::stop_token token) const{
        std::uint64_t bits=0;
        const auto mask=(std::uint32_t(1)<<std::min(record_.idLength,MaxFreeIdBits))-1;
        for(const auto& zone:zones_){
            auto link=getBits(zone.data,FreeLinkBit,mask);std::int64_t start=FreeLinkBit;
            while(link!=0){
                checkCancellation(token);start+=link;
                if(start<zone.startBit||start>=zone.endBit)break;
                const auto fragment=getBits(zone.data,start,mask);
                const auto end=findNextSetBit(zone.data,start+record_.idLength,zone.endBit);
                if(end>=zone.endBit)break;
                bits+=std::uint64_t(end+1-start);
                if(fragment<std::uint32_t(record_.idLength+1))break;
                link=fragment;
            }
        }
        const auto scale=std::uint64_t(1)<<record_.log2BytesPerMapBit;
        if(bits>std::numeric_limits<std::uint64_t>::max()/scale)throw MediaError(DiagnosticCode::FileCoreOverflow);
        return std::min(capacity_,bits*scale);
    }
public:
    NewMap(DiscRecord record,std::vector<Zone> zones,const SectorImage& image,std::stop_token token):record_(std::move(record)),zones_(std::move(zones)),capacity_(image.capacity()),blockCount_(image.blockCount()),blockSize_(image.geometry().blockSize),shift_(record_.log2BytesPerMapBit-record_.log2SectorSize),idsPerZone_(record_.zoneSizeBits()/(record_.idLength+1)){free_=countFree(token);}
    std::int32_t rootAddress() const noexcept override{return record_.rootAddress;}
    const std::string& volumeName() const noexcept override{return record_.discName;}
    std::uint64_t freeBytes() const noexcept override{return free_;}
    std::optional<std::uint64_t> resolve(std::int32_t indirect,std::int64_t offset,std::stop_token token) const override{
        checkCancellation(token);if(indirect<=0||offset<0||idsPerZone_<=0)return {};
        const auto logical=narrow(offset/blockSize_);auto block=logical;
        const auto address=decodeAddress(indirect);
        if(address.shareOffset!=0)block=narrow(std::int64_t(block)+((std::int64_t(address.shareOffset)-1)<<record_.log2ShareSize));
        const auto zoneIndex=address.fragmentId==RootFragment?zones_.size()>>1:address.fragmentId/std::uint32_t(idsPerZone_);
        if(zoneIndex>=zones_.size())return {};
        const auto mapOffset=applyShift(block,-shift_);if(mapOffset<0)return {};
        auto remaining=mapOffset;
        for(std::size_t scanned=0;scanned<zones_.size();++scanned){
            const auto& zone=zones_[(zoneIndex+scanned)%zones_.size()];std::int32_t found=0;
            if(!lookup(zone,address.fragmentId,remaining,found,token))continue;
            const auto sectorOffset=block-applyShift(mapOffset,shift_);
            const auto resolved=narrow(std::int64_t(sectorOffset)+applyShift(narrow(std::int64_t(found)-zone.startBit+zone.startMapBit),shift_));
            if(resolved<=0||std::uint64_t(resolved)>=blockCount_)return {};
            const auto physical=std::uint64_t(resolved)*blockSize_+std::uint64_t(offset%blockSize_);
            if(physical>=capacity_)return {};return physical;
        }
        return {};
    }
};
std::unique_ptr<AddressResolver> mapCopy(const SectorImage& image,const DiscRecord& locator,std::int32_t address,std::int64_t copy,std::stop_token token){
    if(copy<0||std::uint64_t(copy)>image.blockCount()||std::uint64_t(locator.zoneCount)>image.blockCount()-std::uint64_t(copy))return {};
    const auto record=mapRecord(image,std::uint64_t(copy),token);
    if(!record||record->zoneCount!=locator.zoneCount||(std::uint32_t(1)<<record->log2SectorSize)!=image.geometry().blockSize||std::uint64_t(address)+record->zoneCount>image.blockCount()||mapAddress(*record)!=address)return {};
    const auto described=image.capacity()>>record->log2BytesPerMapBit;
    if(described>std::uint64_t(std::numeric_limits<std::int32_t>::max()))throw MediaError(DiagnosticCode::FileCoreOverflow);
    std::vector<Zone> zones;zones.reserve(std::size_t(record->zoneCount));int cross=0;
    for(int index=0;index<record->zoneCount;++index){
        checkCancellation(token);const auto blockNumber=std::uint64_t(copy)+index;
        if(!image.containsBlock(blockNumber))return {};
        const auto block=image.block(blockNumber,token);
        if(!block||block->data.size()!=image.geometry().blockSize)return {};
        const auto& bytes=block->data;
        if((bytes[ZoneTerminatorOffset]&ZoneTerminatorMask)==0||zoneCheck(bytes)!=bytes[0])return {};
        auto zone=createZone(bytes,index,*record,std::int32_t(described));if(!zone)return {};
        cross^=bytes[ZoneCrossCheckOffset];zones.push_back(std::move(*zone));
    }
    if(cross!=CrossCheck)return {};
    return std::make_unique<NewMap>(*record,std::move(zones),image,token);
}
}
std::unique_ptr<AddressResolver> newMap(const SectorImage& image,std::stop_token token){
    checkCancellation(token);auto record=bootRecord(image,token);if(!record)record=mapRecord(image,0,token);
    if(!record||(std::uint32_t(1)<<record->log2SectorSize)!=image.geometry().blockSize)return {};
    const auto address=mapAddress(*record);if(address<0||std::uint64_t(address)>=image.blockCount())return {};
    for(int copy=0;copy<PairCount;++copy)if(auto map=mapCopy(image,*record,address,std::int64_t(address)+std::int64_t(copy)*record->zoneCount,token))return map;
    return {};
}
}
