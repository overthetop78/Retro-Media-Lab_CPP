#include "RawWriter.h"
#include "Geometry.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine::Epson {
void writeRaw(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view format,std::stop_token token){
    MediaFileSystems::checkCancellation(token);const auto& g=resolve(format);const auto& source=image.geometry();
    if(!Primitives::ordinalIdentifierEqualsIgnoreCase(source.formatId,format)||source.cylinders!=std::uint32_t(g.cylinders)||source.heads!=std::uint32_t(g.heads))throw MediaError(DiagnosticCode::EpsonWriteGeometry,{std::string(format)});
    using Address=std::tuple<std::int64_t,std::int64_t,std::int64_t>;std::map<Address,std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;
    image.visitAvailableBlocks([&](auto logical){MediaFileSystems::checkCancellation(token);const auto block=MediaFileSystems::requireSectorBlock(image,logical,token);const auto& a=block->address;if(!blocks.emplace(Address{a.cylinder,a.head,a.number},block).second)throw MediaError(DiagnosticCode::EpsonWriteDuplicate,{std::to_string(a.cylinder),std::to_string(a.head),std::to_string(a.number)});return true;},token);
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){
        for(std::int32_t cylinder=0;cylinder<g.cylinders;++cylinder)for(std::int32_t head=0;head<g.heads;++head){const auto track=g.track(cylinder,head);for(std::int32_t index=0;index<track.count;++index){
            MediaFileSystems::checkCancellation(stop);const auto number=track.firstSector+index;const auto found=blocks.find({cylinder,head,number});auto args=std::vector<std::string>{std::to_string(cylinder),std::to_string(head),std::to_string(number)};
            if(found==blocks.end())throw MediaError(DiagnosticCode::EpsonWriteMissing,args);
            if(found->second->data.size()!=std::size_t(track.sectorSize)){args.push_back(std::to_string(found->second->data.size()));args.push_back(std::to_string(track.sectorSize));throw MediaError(DiagnosticCode::EpsonWriteSize,std::move(args));}
            output(found->second->data);
        }}
    },token);
}
}
