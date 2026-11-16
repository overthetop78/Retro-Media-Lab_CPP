#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <RetroMediaLab/MediaFileSystems/AppleRecognition.h>
#include <set>
namespace RetroMediaLab::MediaFileSystems::AppleDos {
using namespace Constants;
std::optional<Internal::Vtoc> Internal::vtoc(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto& geometry=image.geometry();const auto sectors=geometry.sectorsPerTrack;
    if(geometry.blockSize!=SectorSize||(sectors!=Dos32Sectors&&sectors!=Dos33Sectors)||geometry.cylinders<TrackCount)return {};
    const auto block=image.block(std::uint64_t(VtocTrack)*sectors,token);if(!block||block->data.size()!=SectorSize)return {};
    const auto& data=block->data;const auto tracks=data[Tracks];
    if(!tracks||tracks>geometry.cylinders||!Apple::validDosVtoc(data,tracks,std::int32_t(sectors)))return {};
    std::int32_t free=0;
    for(std::int32_t track=0;track<tracks;++track){
        const auto offset=FreeBitmap+std::size_t(track)*BitmapSize;if(offset>data.size()-BitmapSize)break;
        const auto bits=Binary::big<std::uint32_t>(data,offset);
        for(std::uint32_t sector=0;sector<sectors;++sector)if(bits&(std::uint32_t(1)<<(BitmapBits-sectors+sector)))++free;
    }
    return Vtoc{data,tracks,std::int32_t(sectors),data[CatalogTrack],data[CatalogSector],free,data[VolumeNumber]};
}
Internal::FileData Internal::file(const SectorImage& image,std::int32_t track,std::int32_t sector,std::vector<Diagnostic>& warnings,std::string_view name,std::stop_token token){
    FileData result;const auto& geometry=image.geometry();
    if(validAddress(track,sector,geometry.cylinders,geometry.sectorsPerTrack))result.reference=std::int64_t(track)*geometry.sectorsPerTrack+sector;
    std::set<std::uint64_t> visited;
    const auto warn=[&](DiagnosticCode code,std::int32_t t,std::int32_t s){warnings.push_back({code,{std::string(name),std::to_string(t),std::to_string(s)}});result.valid=false;};
    while(track!=0){
        checkCancellation(token);
        if(!validAddress(track,sector,geometry.cylinders,geometry.sectorsPerTrack)){warn(DiagnosticCode::AppleDosAddress,track,sector);break;}
        const auto logical=std::uint64_t(track)*geometry.sectorsPerTrack+sector;
        if(!visited.insert(logical).second){warn(DiagnosticCode::AppleDosListCycle,track,sector);break;}
        const auto list=image.block(logical,token);
        if(!list||list->data.size()!=SectorSize){warn(DiagnosticCode::AppleDosListMissing,track,sector);break;}
        ++result.traversed;const auto& data=list->data;
        for(std::size_t pair=0;pair<PairCount;++pair){
            const auto offset=PairOffset+pair*PairSize;const auto t=data[offset],s=data[offset+EntrySector];if(!t)continue;
            ++result.traversed;
            if(!validAddress(t,s,geometry.cylinders,geometry.sectorsPerTrack)){warn(DiagnosticCode::AppleDosAddress,t,s);continue;}
            const auto block=image.block(std::uint64_t(t)*geometry.sectorsPerTrack+s,token);
            if(!block){warn(DiagnosticCode::AppleDosDataMissing,t,s);continue;}
            result.content.insert(result.content.end(),block->data.begin(),block->data.end());
        }
        track=data[NextTrack];sector=data[NextSector];
    }
    return result;
}
bool canRead(const SectorImage& image,std::stop_token token){return Internal::vtoc(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    const auto vtoc=Internal::vtoc(image,token);if(!vtoc)throw MediaError(DiagnosticCode::AppleDosCatalogMissing,{std::to_string(-1),std::to_string(-1)});
    FileSystemVolume volume;volume.name=volumeName(vtoc->number);volume.fileSystemId=Id;applyFileSystemDisplayName(volume,image.geometry());volume.capacity=image.capacity();volume.freeBytes=std::uint64_t(vtoc->freeSectors)*SectorSize;
    std::set<std::uint64_t> visited;auto track=vtoc->catalogTrack,sector=vtoc->catalogSector;
    while(track!=0){
        checkCancellation(token);
        if(!Internal::validAddress(track,sector,std::uint32_t(vtoc->tracks),std::uint32_t(vtoc->sectors))){volume.warnings.push_back({DiagnosticCode::AppleDosAddress,{std::string(CatalogName),std::to_string(track),std::to_string(sector)}});break;}
        const auto logical=std::uint64_t(track)*vtoc->sectors+sector;
        if(!visited.insert(logical).second){volume.warnings.push_back({DiagnosticCode::AppleDosCatalogChain,{std::to_string(track),std::to_string(sector)}});break;}
        const auto catalog=image.block(logical,token);
        if(!catalog||catalog->data.size()!=SectorSize){volume.warnings.push_back({DiagnosticCode::AppleDosCatalogChain,{std::to_string(track),std::to_string(sector)}});break;}
        const auto& bytes=catalog->data;
        for(std::size_t index=0;index<EntriesPerSector;++index){
            const auto offset=CatalogFirst+index*EntrySize;const auto tsTrack=bytes[offset+EntryTrack];if(tsTrack==Unused)break;if(tsTrack==Deleted)continue;
            const auto tsSector=bytes[offset+EntrySector],rawType=bytes[offset+EntryType],type=std::uint8_t(rawType&ValueMask);
            auto name=decodeName(Bytes(bytes).subspan(offset+EntryName,NameLength));const auto declared=Binary::little<std::uint16_t>(bytes,offset+EntryCount);
            auto data=Internal::file(image,tsTrack,tsSector,volume.warnings,name,token);const bool sizeValid=declared==data.traversed;
            if(!sizeValid)volume.warnings.push_back({DiagnosticCode::AppleDosSize,{name,std::to_string(declared),std::to_string(data.traversed)}});
            std::uint32_t attributes=rawType;
            if(type==BinaryType)if(auto decoded=decodeBinary(data.content)){data.content=std::move(decoded->content);attributes|=std::uint32_t(decoded->loadAddress)<<BinaryAddressShift;}
            auto typeIndex=std::size_t(std::find(Types.begin(),Types.end(),type)-Types.begin());
            FileSystemEntry entry;entry.name=std::move(name);entry.kind=EntryKind::File;entry.size=data.content.size();entry.comment=TypeNames[typeIndex];entry.commentResourceKey=TypeKeys[typeIndex];entry.commentTranslationDomain=Domain;
            entry.rawAttributes=attributes;entry.storageReference=data.reference;entry.metadataValid=data.valid&&sizeValid;entry.content=std::move(data.content);volume.entries.push_back(std::move(entry));
        }
        track=bytes[NextTrack];sector=bytes[NextSector];
    }
    const auto upper=[](char c){return c>=LowerFirst&&c<=LowerLast?char(c-CaseDelta):c;};
    std::stable_sort(volume.entries.begin(),volume.entries.end(),[&](const auto& a,const auto& b){return std::lexicographical_compare(a.name.begin(),a.name.end(),b.name.begin(),b.name.end(),[&](char x,char y){return upper(x)<upper(y);});});
    return volume;
}
}
