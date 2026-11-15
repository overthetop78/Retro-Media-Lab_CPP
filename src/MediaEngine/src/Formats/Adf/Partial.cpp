#include "Internal.h"
#include <algorithm>
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Adf {
using namespace Constants;
std::optional<PartialDescription> describePartial(MediaFileSystems::Bytes header,std::uint64_t stored){
    if(header.size()<OldMapLength||stored>=HighCapacity)return {};
    if(const auto declared=MediaFileSystems::FileCore::oldMapDeclaredCapacity(header)){
        auto profile=*findGeometry(AcornAdfs800);
        for(const auto& candidate:acornGeometries())if(candidate.geometry.blockSize==BbcBlockSize&&Internal::capacity(candidate)==*declared){profile=candidate;break;}
        if(*declared>stored&&*declared<=Internal::capacity(profile)&&*declared%profile.geometry.blockSize==0)return PartialDescription{profile,*declared};
    }
    namespace Layout=MediaFileSystems::FileCore::Layout;
    const auto recordAt=[&](std::size_t offset)->std::optional<PartialDescription>{
        if(offset>header.size()||Layout::DiscRecordLength>header.size()-offset)return {};
        const auto record=MediaFileSystems::FileCore::parseDiscRecord(header.subspan(offset,Layout::DiscRecordLength),HighCapacity);
        if(!record||record->log2SectorSize!=ArchimedesSectorLog||record->discSize<=stored||record->discSize%ArchimedesBlockSize!=0)return {};
        return PartialDescription{*findGeometry(record->discSize>Capacity?AcornAdfs1600:AcornAdfs800),record->discSize};
    };
    if(auto result=recordAt(Layout::DiscRecordOffset))return result;
    return recordAt(Layout::BootBlockAddress+Layout::BootDiscRecordOffset);
}
namespace Internal {
std::vector<std::uint8_t> header(const MediaFileSystems::RandomAccessData& source,std::stop_token token){checkCancellation(token);std::vector<std::uint8_t> bytes(std::size_t(std::min(std::uint64_t(ProbeLength),source.length())));source.readExactly(0,bytes,token);return bytes;}
std::shared_ptr<const MediaFileSystems::SectorImage> image(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,const Profile& profile,std::optional<std::uint64_t> declared){
    MediaFileSystems::SectorImageOptions options;
    if(declared){options.capacity=*declared;options.logicalBlockCount=*declared/profile.geometry.blockSize;}
    SourceSectorMetadata metadata;metadata.firstSectorNumber=profile.firstSectorNumber;metadata.trustCompletePayload=true;
    if(profile.geometry.blockSize==BbcBlockSize)metadata.recordedHead=0;
    return sourceSectorImage(std::move(source),profile.geometry,0,options,metadata);
}
std::shared_ptr<const MediaFileSystems::SectorImage> partial(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,MediaFileSystems::Bytes header,std::stop_token token){
    const auto description=describePartial(header,source->length());if(!description)return {};
    auto candidate=image(std::move(source),description->profile,description->capacity);
    if(!MediaFileSystems::Adfs::canRead(*candidate,token))return {};return candidate;
}
}
}
