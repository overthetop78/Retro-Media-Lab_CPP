#include "Automatic.h"
#include "RuntimeConstants.h"
#include "../../Formats/Atari8Bit/Constants.h"
#include "../../Formats/St/Geometry.h"
#include "../../Formats/St/Constants.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Raw/Constants.h"
#include "../../Formats/BbcDfs/BbcDfsImages.h"
#include "../../Formats/BbcDfs/Constants.h"
#include "../../Formats/Epson/Geometry.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <limits>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace M=MediaFileSystems;
namespace {
std::int32_t indexOf(const Measurement& m,M::SectorAddress address){if(!m.zeroBased)return static_cast<std::int32_t>(address.number-1);const auto found=std::find(m.sectorOrder.begin(),m.sectorOrder.end(),address.number);return found==m.sectorOrder.end()?-1:static_cast<std::int32_t>(found-m.sectorOrder.begin());}
std::int64_t capacity(std::int32_t c,std::int32_t h,std::int32_t s,std::int32_t size,std::int64_t maximum=std::numeric_limits<std::int64_t>::max()){std::int64_t result=1;for(auto value:{c,h,s,size}){if(value<=0||result>maximum/value)throw MediaError(DiagnosticCode::IsoGeometryOverflow,{std::to_string(c),std::to_string(h),std::to_string(s)});result*=value;}return result;}
UniformRequest request(std::string format,const Measurement& m,std::int32_t c,std::int32_t h,std::int32_t s,std::int32_t size,bool normalize=false){UniformRequest r;r.formatId=std::move(format);r.cylinders=c;r.heads=h;r.sectorsPerTrack=s;r.sectorSize=size;r.sectorIndex=[m](auto address){return indexOf(m,address);};if(normalize)r.normalizeData=[size](auto data){return padTo(data,size);};return r;}
}
std::shared_ptr<const M::SectorImage> buildAtari8Bit(const CandidateSet& set,std::optional<std::string> format,std::stop_token token){
    namespace C=Atari8Bit::Constants;const auto m=measure(set.addressed,token);std::string resolved;
    if(format)resolved=*format;else if(m.sectorSize==C::SingleSize&&m.sectorsPerTrack==C::StandardSectors)resolved=C::Atari90;
    else if(m.sectorSize==C::SingleSize&&m.sectorsPerTrack==C::EnhancedSectors)resolved=C::Atari130;
    else if(m.sectorSize==C::DoubleSize&&m.sectorsPerTrack==C::StandardSectors)resolved=C::Atari180;
    else resolved=std::string(RuntimeConstants::AtariScpPrefix)+std::to_string(m.sectorSize)+RuntimeConstants::Separator+std::to_string(m.sectorsPerTrack);
    auto r=request(std::move(resolved),m,m.cylinders,m.heads,m.sectorsPerTrack,m.sectorSize);r.allowVariableBlockSize=m.sectorSize>C::SingleSize;
    const auto total=capacity(m.cylinders,m.heads,m.sectorsPerTrack,m.sectorSize);
    r.capacity=r.allowVariableBlockSize?total-std::int64_t(C::BootCount)*(m.sectorSize-C::SingleSize):total;return createUniform(set.addressed,r,token);
}
std::shared_ptr<const M::SectorImage> buildSt(const CandidateSet& set,std::optional<std::string> format,std::stop_token token){
    const auto m=measure(set.addressed,token);const auto explicitGeometry=format?St::geometry(*format):std::nullopt;const auto bpb=!explicitGeometry?M::Fat::detectBpbGeometry(bestData(set.addressed,{0,0,1},token)):std::nullopt;
    const auto c=explicitGeometry?static_cast<std::int32_t>(explicitGeometry->cylinders):bpb?static_cast<std::int32_t>(bpb->cylinders):m.cylinders;
    const auto h=explicitGeometry?static_cast<std::int32_t>(explicitGeometry->heads):bpb?static_cast<std::int32_t>(bpb->heads):m.heads;
    const auto s=explicitGeometry?static_cast<std::int32_t>(explicitGeometry->sectorsPerTrack):bpb?static_cast<std::int32_t>(bpb->sectorsPerTrack):m.sectorsPerTrack;
    const auto size=explicitGeometry||bpb?static_cast<std::int32_t>(St::Constants::SectorSize):m.sectorSize;
    return createUniform(set.addressed,request(explicitGeometry?explicitGeometry->formatId:St::formatId(capacity(c,h,s,size)),m,c,h,s,size,explicitGeometry.has_value()||bpb.has_value()),token);
}
std::shared_ptr<const M::SectorImage> buildBbc(const CandidateSet& set,std::stop_token token){
    const auto m=measure(set.physical,token);const auto g=BbcDfs::observedGeometry(m.cylinders,m.heads);auto r=request(g.formatId,m,static_cast<std::int32_t>(g.cylinders),static_cast<std::int32_t>(g.heads),BbcDfs::Constants::SectorsPerTrack,m.sectorSize);
    r.sectorIndex=[order=m.sectorOrder](auto address){const auto found=std::find(order.begin(),order.end(),address.number);return found==order.end()?-1:static_cast<std::int32_t>(found-order.begin());};return createUniform(set.physical,r,token);
}
std::shared_ptr<const M::SectorImage> buildIbmAutomatic(const CandidateSet& set,std::stop_token token){
    const auto m=measure(set.addressed,token);const auto boot=bestData(set.addressed,{0,0,1},token);const auto bpb=!m.zeroBased?M::Fat::detectBpbGeometry(boot):std::nullopt;auto c=m.cylinders,h=m.heads,s=m.sectorsPerTrack,size=m.sectorSize;
    if(bpb){c=static_cast<std::int32_t>(bpb->cylinders);h=static_cast<std::int32_t>(bpb->heads);s=static_cast<std::int32_t>(bpb->sectorsPerTrack);size=static_cast<std::int32_t>(bpb->sectorSize);}
    else if(size==Raw::Constants::SectorSize&&!m.zeroBased){const auto fat=bestData(set.addressed,{0,0,RuntimeConstants::FatFirstSector},token);if(const auto g=Raw::ibmFromBoot(boot,fat.empty()?0:fat.front())){c=static_cast<std::int32_t>(g->cylinders);h=static_cast<std::int32_t>(g->heads);s=static_cast<std::int32_t>(g->sectorsPerTrack);}}
    const auto length=capacity(c,h,s,size,std::numeric_limits<std::int32_t>::max());const auto known=Raw::ibmFromCapacity(static_cast<std::uint64_t>(length));const auto id=known?known->formatId:std::string(Raw::Constants::IbmPrefix)+std::to_string(length/Raw::Constants::BytesPerKibibyte);
    return createUniform(set.addressed,request(id,m,c,h,s,size,bpb.has_value()),token);
}
std::shared_ptr<const M::SectorImage> buildEpson(std::string_view format,const Candidates& candidates,std::stop_token token){
    const auto& g=Epson::resolve(format);std::vector<M::SectorBlock> blocks;std::uint64_t logical=0,bytes=0;std::int32_t maximum=0;std::vector<std::pair<std::int32_t,std::uint64_t>> sizes;
    for(std::int32_t c=0;c<g.cylinders;++c)for(std::int32_t h=0;h<g.heads;++h){checkCancellation(token);const auto t=g.track(c,h);maximum=std::max(maximum,t.count);
        for(std::int32_t i=0;i<t.count;++i,++logical){const M::SectorAddress address{c,h,t.firstSector+i};bytes+=t.sectorSize;auto found=std::find_if(sizes.begin(),sizes.end(),[&](auto value){return value.first==t.sectorSize;});if(found==sizes.end())sizes.emplace_back(t.sectorSize,1);else ++found->second;
            const auto* values=candidates.find(address);if(!values)continue;std::vector<Candidate> matching;for(const auto& value:*values)if(value.sector.data&&value.sector.data->size()==static_cast<std::size_t>(t.sectorSize))matching.push_back(value);if(matching.empty())continue;const auto& chosen=best(matching,token);M::SectorBlock b;b.logicalBlock=logical;b.address=address;b.data=*chosen.sector.data;b.integrityValid=chosen.sector.integrityValid;b.revolution=chosen.revolution;blocks.push_back(std::move(b));
        }
    }
    const auto dominant=std::max_element(sizes.begin(),sizes.end(),[](const auto& a,const auto& b){return a.second<b.second;});if(dominant==sizes.end())throw MediaError(DiagnosticCode::IsoEmptyCandidates);
    M::SectorImageOptions options;options.allowVariableBlockSize=sizes.size()>1;options.capacity=bytes;options.logicalBlockCount=logical;return sparseSectorImage({std::string(format),static_cast<std::uint32_t>(dominant->first),static_cast<std::uint32_t>(g.cylinders),static_cast<std::uint32_t>(g.heads),static_cast<std::uint32_t>(maximum)},std::move(blocks),options);
}
}
