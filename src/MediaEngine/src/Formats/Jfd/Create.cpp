#include "Jfd.h"
#include "Binary.h"
#include "../../Reconstruction/Sector/Registry.h"
#include "../../Models/Sectors/SizeCodes.h"
#include "../../Codecs/Iso/FmConstants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include <algorithm>
#include <map>
#include <set>
#include <tuple>
namespace RetroMediaLab::MediaEngine::Jfd {
namespace {
using namespace Constants;
using Block=MediaFileSystems::SectorBlock;
using SectorImage=MediaFileSystems::SectorImage;
using Blocks=std::vector<std::shared_ptr<const Block>>;
using Policy=Reconstruction::Sector::Registry::Entry;
bool prepare(const SectorImage& image,Blocks& blocks,Policy& policy,std::stop_token token) {
    const auto& g=image.geometry();
    if(g.addressing!=MediaFileSystems::SectorAddressingKind::Physical||g.cylinders>MaximumCylinders||g.heads>Heads)return false;
    image.visitAvailableBlocks([&](auto logical){blocks.push_back(MediaFileSystems::requireSectorBlock(image,logical,token));return true;},token);
    if(blocks.size()!=std::uint64_t(g.cylinders)*g.heads*g.sectorsPerTrack)return false;
    policy=Reconstruction::Sector::Registry{}.resolve(image);if(!policy)return false;
    const auto id=policy->encoderId(image);if(id!=IsoFm::Constants::Id&&id!=IsoMfm::Constants::Id)return false;
    std::set<std::tuple<std::int64_t,std::int64_t,std::int64_t>> addresses;
    for(const auto& block:blocks){checkCancellation(token);const auto& a=block->address;const auto code=trySectorSizeCode(std::int64_t(block->data.size()));
        if(a.cylinder<0||a.cylinder>=g.cylinders||a.head<0||a.head>=g.heads||a.number<0||a.number>MaximumSectorNumber||block->deletedDataMark==true
            ||(block->recordedAddress&&*block->recordedAddress!=a)||!code||*code>CreationSizeCode||(block->recordedSizeCode&&block->recordedSizeCode!=code))return false;
        if(!addresses.emplace(a.cylinder,a.head,a.number).second)return false;
    }return true;
}
}
bool canCreate(const MediaFileSystems::SectorImage& image,std::stop_token token) {
    Blocks blocks;Policy policy;return prepare(image,blocks,policy,token);
}
std::vector<std::uint8_t> create(const MediaFileSystems::SectorImage& image,std::stop_token token) {
    using namespace Constants;Blocks blocks;Policy policy;
    if(!prepare(image,blocks,policy,token))throw MediaError(DiagnosticCode::JfdCreate);
    const auto count=std::max(MinimumCreationTracks,image.geometry().cylinders*Heads);
    std::map<std::uint32_t,Blocks> groups;std::size_t payloadBytes{};
    for(const auto& block:blocks){groups[std::uint32_t(block->address.cylinder*Heads+block->address.head)].push_back(block);payloadBytes+=block->data.size();}
    const auto sectorOffset=CreationTrackOffset+count*FieldBytes;std::size_t dataOffset=sectorOffset;
    for(auto& [track,items]:groups){dataOffset+=items.size()*DescriptorBytes+FieldBytes;
        std::stable_sort(items.begin(),items.end(),[](const auto& a,const auto& b){return a->recordedTrackOrder.value_or(std::int32_t(a->address.number))<b->recordedTrackOrder.value_or(std::int32_t(b->address.number));});
    }
    std::vector<std::uint8_t> bytes(dataOffset+payloadBytes);std::copy(Signature.begin(),Signature.end(),bytes.begin());
    const auto put=[&](auto offset,auto value){Binary::put(bytes,offset,std::uint32_t(value));};
    put(HeaderSizeOffset,CreationHeaderSize);put(LengthOffset,bytes.size());put(VersionOffset,CreationVersion);
    put(TracksOffset,CreationTrackOffset);put(SectorsOffset,sectorOffset);put(DataOffset,dataOffset);
    auto recordAt=std::size_t(sectorOffset),dataAt=dataOffset;
    for(std::uint32_t track=0;track<count;++track){checkCancellation(token);const auto found=groups.find(track);
        if(found==groups.end()){put(CreationTrackOffset+track*FieldBytes,Absent);continue;}
        put(CreationTrackOffset+track*FieldBytes,recordAt-sectorOffset);
        const auto ticks=policy->bitCellTicks(image,std::int32_t(track/Heads));const auto density=ticks==0?0:DensityTicks/ticks;
        if(std::find(Densities.begin(),Densities.end(),density)==Densities.end())throw MediaError(DiagnosticCode::JfdSectorEncoding);
        for(const auto& block:found->second){auto header=DefaultPosition|(density<<DensityShift)|(std::uint32_t(block->address.number)<<NumberShift)|*trySectorSizeCode(std::int64_t(block->data.size()));
            if(block->headerIntegrityValid==false)header|=HeaderError;
            if(block->dataIntegrityValid.value_or(block->integrityValid.value_or(true))==false)header|=DataError;
            put(recordAt,header);put(recordAt+FieldBytes,dataAt-dataOffset);std::copy(block->data.begin(),block->data.end(),bytes.begin()+dataAt);dataAt+=block->data.size();recordAt+=DescriptorBytes;
        }put(recordAt,Absent);recordAt+=FieldBytes;
    }return bytes;
}
}
