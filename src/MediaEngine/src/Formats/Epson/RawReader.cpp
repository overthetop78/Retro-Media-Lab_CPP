#include "RawReader.h"
#include "Geometry.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine::Epson {
namespace {
std::int64_t expectedLength(const Geometry& geometry){std::int64_t result=0;for(const auto& track:geometry.allTracks())result+=std::int64_t(track.count)*track.sectorSize;return result;}
const Geometry* find(std::string_view id){const auto& all=geometries();const auto it=std::find_if(all.begin(),all.end(),[&](const auto& g){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,g.formatId);});return it==all.end()?nullptr:&*it;}
}
const ReaderCapabilities& RawReader::capabilities()const{static const auto result=[]{ReaderCapabilities c;for(const auto& g:geometries())c.formatIds.push_back(g.formatId);c.extensions={std::string(Constants::RawExtension)};c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return result;}
bool RawReader::supportsFormatId(std::string_view id)const{return find(id)!=nullptr;}
std::future<bool> RawReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{MediaFileSystems::checkCancellation(token);const auto requested=context.requestedFormatId();const auto* geometry=requested?find(*requested):nullptr;return geometry&&context.length()==expectedLength(*geometry);});}
std::future<std::shared_ptr<const MediaDocument>> RawReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto requested=context.requestedFormatId();if(!requested)throw MediaError(DiagnosticCode::EpsonRawExplicit);
    const auto bytes=context.readBytesAsync(token).get();const auto& geometry=resolve(*requested);const auto expected=expectedLength(geometry);
    if(std::int64_t(bytes.size())!=expected)throw MediaError(DiagnosticCode::EpsonRawLength,{std::to_string(bytes.size()),std::to_string(expected),*requested});
    std::vector<MediaFileSystems::SectorBlock> blocks;std::set<std::int32_t> sizes;std::vector<std::pair<std::int32_t,std::int64_t>> counts;std::size_t offset=0;std::int32_t maximum=0;
    for(std::int32_t cylinder=0;cylinder<geometry.cylinders;++cylinder)for(std::int32_t head=0;head<geometry.heads;++head){
        const auto track=geometry.track(cylinder,head);maximum=std::max(maximum,track.count);sizes.insert(track.sectorSize);
        auto frequency=std::find_if(counts.begin(),counts.end(),[&](const auto& p){return p.first==track.sectorSize;});if(frequency==counts.end())counts.emplace_back(track.sectorSize,track.count);else frequency->second+=track.count;
        for(std::int32_t index=0;index<track.count;++index){MediaFileSystems::checkCancellation(token);MediaFileSystems::SectorBlock block;block.logicalBlock=blocks.size();block.address={cylinder,head,track.firstSector+index};block.data.assign(bytes.begin()+std::ptrdiff_t(offset),bytes.begin()+std::ptrdiff_t(offset+track.sectorSize));blocks.push_back(std::move(block));offset+=std::size_t(track.sectorSize);}
    }
    const auto dominant=std::max_element(counts.begin(),counts.end(),[](const auto& a,const auto& b){return a.second<b.second;})->first;
    const auto count=blocks.size();MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=sizes.size()>1;options.capacity=bytes.size();options.logicalBlockCount=count;
    auto image=sparseSectorImage({*requested,std::uint32_t(dominant),std::uint32_t(geometry.cylinders),std::uint32_t(geometry.heads),std::uint32_t(maximum)},std::move(blocks),options);
    return floppySectorDocument(context.source(),std::move(image));
});}
}
