#include "Internal.h"
#include "../../../Primitives/Identifiers.h"
#include <numeric>
#include <cstdlib>
#include <QList>
namespace RetroMediaLab::MediaFileSystems::AppleDos {
using namespace Constants;
namespace {
using Sectors=std::vector<std::vector<std::uint8_t>>;
void address(std::vector<std::uint8_t>& bytes,std::size_t offset,std::uint32_t logical,std::uint32_t sectors){bytes[offset]=std::uint8_t(logical/sectors);bytes[offset+1]=std::uint8_t(logical%sectors);}
std::vector<std::uint8_t> nameBytes(std::string_view name){
    const auto decoded=Identifiers::utf8(name);std::vector<std::uint8_t> encoded;encoded.reserve(decoded.size());
    for(auto c:decoded)encoded.push_back(c.unicode()<=ValueMask?std::uint8_t(c.unicode()):AsciiReplacement);
    if(encoded.size()>NameLength)throw MediaError(DiagnosticCode::AppleDosWriteName,{std::string(name)});
    std::vector<std::uint8_t> result(NameLength,NamePadding);for(std::size_t i=0;i<encoded.size();++i)result[i]=std::uint8_t(encoded[i]|HighBit);return result;
}
std::vector<std::uint32_t> allocate(std::vector<bool>& free,std::uint64_t count,std::uint32_t sectors){
    std::vector<std::int32_t> tracks(TrackCount);std::iota(tracks.begin(),tracks.end(),0);
    std::sort(tracks.begin(),tracks.end(),[](auto a,auto b){const auto da=std::abs(a-(VtocTrack-1)),db=std::abs(b-(VtocTrack-1));return da==db?a<b:da<db;});
    std::vector<std::uint32_t> result;
    for(auto track:tracks)for(std::uint32_t sector=0;sector<sectors;++sector){const auto logical=std::uint32_t(track)*sectors+sector;if(free[logical]&&result.size()<count)result.push_back(logical);}
    if(result.size()!=count)throw MediaError(DiagnosticCode::AppleDosWriteFull);
    for(auto logical:result)free[logical]=false;return result;
}
}
SectorWritePlan write(const MigrationPlan& plan,std::string_view formatId,std::stop_token token){
    checkCancellation(token);std::uint32_t sectors=0;
    if(Internal::sameId(formatId,Dos32)||Internal::sameId(formatId,Dos113))sectors=Dos32Sectors;
    else if(Internal::sameId(formatId,Dos33)||Internal::sameId(formatId,Dos140))sectors=Dos33Sectors;
    else throw MediaError(DiagnosticCode::AppleDosWriteFormat,{std::string(formatId)});
    const auto volume=volumeNumber(plan.volumeName);if(!volume)throw MediaError(DiagnosticCode::AppleDosWriteVolume,{plan.volumeName});
    for(const auto& entry:plan.entries)if(entry.kind!=EntryKind::File||!entry.children.empty()||!entry.content)throw MediaError(DiagnosticCode::AppleDosWriteEntry);
    const auto capacity=TrackCount*sectors;Sectors data(capacity,std::vector<std::uint8_t>(SectorSize));std::vector<bool> free(capacity,true);
    for(std::uint32_t sector=0;sector<sectors;++sector){free[sector]=false;free[VtocTrack*sectors+sector]=false;}
    for(std::uint32_t sector=sectors-1;sector>0;--sector){auto& catalog=data[VtocTrack*sectors+sector];catalog[NextTrack]=sector==1?0:std::uint8_t(VtocTrack);catalog[NextSector]=sector==1?0:std::uint8_t(sector-1);}
    if(plan.entries.size()>(sectors-1)*EntriesPerSector)throw MediaError(DiagnosticCode::AppleDosWriteFull);
    for(std::size_t index=0;index<plan.entries.size();++index){
        checkCancellation(token);const auto& entry=plan.entries[index];const auto rawType=Internal::sameId(plan.sourceFileSystemId,Id)?std::uint8_t(entry.rawAttributes):BinaryType;
        auto content=*entry.content;
        if((rawType&ValueMask)==BinaryType){const auto load=entry.rawAttributes>>BinaryAddressShift;if(load>MaximumBinarySize)throw MediaError(DiagnosticCode::AppleDosLoadAddress,{std::to_string(load)});content=encodeBinary(content,std::uint16_t(load));}
        const auto dataCount=std::max(std::size_t(1),(content.size()+SectorSize-1)/SectorSize),listCount=(dataCount+PairCount-1)/PairCount;
        const auto allocated=allocate(free,dataCount+listCount,sectors);
        const auto lists=std::span(allocated).first(listCount),payload=std::span(allocated).subspan(listCount);
        for(std::size_t part=0;part<payload.size();++part){const auto offset=part*SectorSize;const auto count=std::min(std::size_t(SectorSize),content.size()>offset?content.size()-offset:0);if(count)std::copy_n(content.begin()+offset,count,data[payload[part]].begin());}
        for(std::size_t listIndex=0;listIndex<lists.size();++listIndex){
            auto& list=data[lists[listIndex]];if(listIndex+1<lists.size())address(list,NextTrack,lists[listIndex+1],sectors);
            Binary::writeLittle<std::uint16_t>(list,ListOffset,std::uint16_t(listIndex*PairCount));const auto first=listIndex*PairCount,count=std::min(PairCount,payload.size()-first);
            for(std::size_t pair=0;pair<count;++pair)address(list,PairOffset+pair*PairSize,payload[first+pair],sectors);
        }
        const auto catalogSector=sectors-1-std::uint32_t(index/EntriesPerSector);auto& catalog=data[VtocTrack*sectors+catalogSector];const auto offset=CatalogFirst+index%EntriesPerSector*EntrySize;
        address(catalog,offset,lists.front(),sectors);catalog[offset+EntryType]=rawType;const auto encoded=nameBytes(entry.targetName);std::copy(encoded.begin(),encoded.end(),catalog.begin()+offset+EntryName);Binary::writeLittle<std::uint16_t>(catalog,offset+EntryCount,std::uint16_t(dataCount+listCount));
    }
    auto& vtoc=data[VtocTrack*sectors];vtoc[0]=sectors==Dos32Sectors?Dos32Version:Dos33Version;vtoc[CatalogTrack]=VtocTrack;vtoc[CatalogSector]=std::uint8_t(sectors-1);vtoc[VolumeNumber]=*volume;vtoc[MaximumPairs]=PairCount;vtoc[LastAllocated]=VtocTrack-1;vtoc[AllocationDirection]=Descending;vtoc[Tracks]=TrackCount;vtoc[TrackSectors]=std::uint8_t(sectors);Binary::writeLittle<std::uint16_t>(vtoc,SectorBytes,SectorSize);
    for(std::uint32_t track=0;track<TrackCount;++track){std::uint32_t bits=0;for(std::uint32_t sector=0;sector<sectors;++sector)if(free[track*sectors+sector])bits|=std::uint32_t(1)<<(BitmapBits-sectors+sector);Binary::writeBig<std::uint32_t>(vtoc,FreeBitmap+track*BitmapSize,bits);}
    SectorWritePlan result;result.geometry={std::string(formatId),SectorSize,TrackCount,1,sectors};result.blocks.reserve(capacity);
    for(std::uint32_t logical=0;logical<capacity;++logical)result.blocks.push_back({logical,{logical/sectors,0,logical%sectors},std::move(data[logical])});return result;
}
}
