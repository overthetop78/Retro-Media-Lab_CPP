#include "UniformLayout.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaEngine {
std::shared_ptr<const MediaFileSystems::SectorImage> tryUniformSectorImage(MediaFileSystems::SectorGeometry geometry,const std::vector<MediaFileSystems::SectorBlock>& blocks,std::stop_token token){
    checkCancellation(token);if(blocks.empty())return {};const auto first=std::min_element(blocks.begin(),blocks.end(),[](const auto& a,const auto& b){return a.address.number<b.address.number;})->address.number;if(first!=0&&first!=1)return {};
    for(const auto& block:blocks){checkCancellation(token);const auto& a=block.address;if(a.cylinder<0||std::uint64_t(a.cylinder)>=geometry.cylinders||a.head<0||std::uint64_t(a.head)>=geometry.heads||a.number<first||a.number>=first+geometry.sectorsPerTrack||block.data.size()>geometry.blockSize)return {};}
    const auto tracks=std::uint64_t(geometry.cylinders)*geometry.heads;if(geometry.sectorsPerTrack!=0&&tracks>std::uint64_t(std::numeric_limits<std::int32_t>::max())/geometry.sectorsPerTrack)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::ReadOutOfBounds,{},MediaFileSystems::ErrorKind::Overflow);
    auto mapped=blocks;std::set<std::uint64_t> used;for(auto& block:mapped){checkCancellation(token);block.logicalBlock=(std::uint64_t(block.address.cylinder)*geometry.heads+std::uint64_t(block.address.head))*geometry.sectorsPerTrack+std::uint64_t(block.address.number-first);if(!used.insert(block.logicalBlock).second)return {};}
    return sparseSectorImage(std::move(geometry),std::move(mapped));
}
}
