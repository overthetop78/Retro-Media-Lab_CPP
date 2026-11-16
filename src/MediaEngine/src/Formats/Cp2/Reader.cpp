#include "Cp2.h"
#include "Constants.h"
#include "../Raw/RawImages.h"
#include "../Raw/Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine::Cp2 {
namespace {
using namespace Constants;
using Address=std::tuple<std::uint8_t,std::uint8_t,std::uint8_t>;
using Sectors=std::map<Address,std::vector<std::uint8_t>>;
Sectors readGroups(std::span<const std::uint8_t> bytes,std::stop_token token) {
    Sectors result;std::size_t group=FirstGroupOffset;
    while(group<=bytes.size()&&GroupHeaderSize<=bytes.size()-group){checkCancellation(token);
        const auto at=group+MetadataLengthOffset;const auto length=std::size_t(bytes[at]|std::uint32_t(bytes[at+1])<<ByteBits);
        if(length==0||(length-MetadataLengthAdjustment)%TrackDescriptorSize!=0)throw MediaError(DiagnosticCode::Cp2Description,{std::to_string(group),std::to_string(length),std::to_string(bytes.size()-group)});
        const auto count=(length-MetadataLengthAdjustment)/TrackDescriptorSize;std::vector<std::vector<SectorDescriptor>> tracks;tracks.reserve(count);
        for(std::size_t index=0;index<count;++index){const auto offset=group+GroupHeaderSize+index*TrackDescriptorSize;
            if(offset>bytes.size()||TrackDescriptorSize>bytes.size()-offset)throw MediaError(DiagnosticCode::Cp2DescriptionTruncated,{std::to_string(offset),std::to_string(TrackDescriptorSize),std::to_string(std::int64_t(bytes.size())-std::int64_t(offset))});
            auto sectors=parseTrack(bytes.subspan(offset,TrackDescriptorSize));if(!sectors.empty())tracks.push_back(std::move(sectors));
        }
        auto payload=group+GroupHeaderSize+length+FramingSize;
        for(auto& track:tracks){std::stable_sort(track.begin(),track.end(),[](const auto& a,const auto& b){return a.position<b.position;});
            for(const auto& sector:track){checkCancellation(token);
                if(payload>bytes.size()||sector.size>bytes.size()-payload)throw MediaError(DiagnosticCode::Cp2Payload,{std::to_string(sector.cylinder),std::to_string(sector.head),std::to_string(sector.number),std::to_string(payload),std::to_string(sector.size),std::to_string(std::int64_t(bytes.size())-std::int64_t(payload))});
                if(sector.size==ReconstructedSectorSize){const Address key{sector.cylinder,sector.head,sector.number};if(!result.contains(key))result.emplace(key,std::vector<std::uint8_t>(bytes.begin()+payload,bytes.begin()+payload+sector.size));}
                payload+=sector.size;
            }
        }
        if(payload>=bytes.size())break;group=payload-FramingSize;
    }return result;
}
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(std::span<const std::uint8_t> bytes,std::stop_token token) {
    using namespace Constants;checkCancellation(token);
    if(bytes.size()<MinimumFileLength||!std::equal(Signature.begin(),Signature.end(),bytes.begin()+SignatureOffset))throw MediaError(DiagnosticCode::Cp2Signature);
    const auto sectors=readGroups(bytes,token);if(sectors.empty())throw MediaError(DiagnosticCode::Cp2NoSectors);
    std::uint32_t cylinders{},heads{},spt{};
    for(const auto& [address,data]:sectors){cylinders=std::max(cylinders,std::uint32_t(std::get<CylinderIndex>(address))+1);heads=std::max(heads,std::uint32_t(std::get<HeadIndex>(address))+1);spt=std::max(spt,std::uint32_t(std::get<NumberIndex>(address)));}
    if(heads==0||heads>MaximumHeads||spt==0)throw MediaError(DiagnosticCode::Cp2Geometry,{std::to_string(heads),std::to_string(spt)});
    const auto capacity=std::uint64_t(cylinders)*heads*spt*ReconstructedSectorSize;std::vector<std::uint8_t> linear(std::size_t(capacity),std::uint8_t{});
    for(const auto& [address,data]:sectors){checkCancellation(token);const auto [c,h,n]=address;if(data.size()!=ReconstructedSectorSize||n==0||n>spt)continue;
        const auto logical=(std::uint64_t(c)*heads+h)*spt+n-1;std::copy(data.begin(),data.end(),linear.begin()+logical*ReconstructedSectorSize);
    }
    const auto known=Raw::ibmFromCapacity(capacity);const auto id=known?known->formatId:std::string(Raw::Constants::IbmPrefix)+std::to_string(capacity/Raw::Constants::BytesPerKibibyte);
    return sourceSectorImage(memorySource(linear),{id,ReconstructedSectorSize,cylinders,heads,spt},0,{},SourceSectorMetadata{1,{},true});
}
}
