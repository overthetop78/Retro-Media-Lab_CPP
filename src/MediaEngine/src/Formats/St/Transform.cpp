#include "Transform.h"
#include "Geometry.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::St {
namespace M=MediaFileSystems;
namespace {
std::uint64_t capacity(const M::SectorGeometry& g){return std::uint64_t(g.cylinders)*g.heads*g.sectorsPerTrack*g.blockSize;}
bool matches(const M::SectorImage& image,const M::SectorGeometry& target){return image.geometry().blockSize==Constants::SectorSize&&image.capacity()==capacity(target)&&image.blockCount()==capacity(target)/Constants::SectorSize;}
std::string dimensions(const M::SectorGeometry& g){const auto separator=std::string(1,Constants::GeometrySeparator);return std::to_string(g.cylinders)+separator+std::to_string(g.heads)+separator+std::to_string(g.sectorsPerTrack);}
}
bool canTransformGeometry(const M::SectorImage& image,std::string_view target){const auto g=geometry(target);return g&&matches(image,*g);}
std::shared_ptr<const M::SectorImage> transform(const M::SectorImage& source,std::string_view targetFormat,bool complete,std::stop_token token){
    M::checkCancellation(token);const auto target=geometry(targetFormat);if(!target)throw MediaError(DiagnosticCode::StTargetFormat,{std::string(targetFormat)});
    if(!matches(source,*target))throw MediaError(DiagnosticCode::StLossyGeometry,{source.geometry().formatId,dimensions(source.geometry()),std::to_string(source.capacity()),target->formatId,dimensions(*target),std::to_string(capacity(*target))});
    std::vector<M::SectorBlock> blocks;blocks.reserve(std::size_t(source.blockCount()));const auto perCylinder=std::uint64_t(target->heads)*target->sectorsPerTrack;
    for(std::uint64_t logical=0;logical<source.blockCount();++logical){
        M::checkCancellation(token);const auto original=source.block(logical,token);M::SectorBlock block;
        if(original)block=*original;else{if(!complete)throw MediaError(DiagnosticCode::StSourceMissing,{std::to_string(logical)});block.logicalBlock=logical;block.data.resize(Constants::SectorSize);}
        if(block.data.size()!=Constants::SectorSize)throw MediaError(DiagnosticCode::StLogicalSize,{std::to_string(logical),std::to_string(block.data.size()),std::to_string(Constants::SectorSize)});
        const auto within=logical%perCylinder;block.address={std::int64_t(logical/perCylinder),std::int64_t(within/target->sectorsPerTrack),std::int64_t(within%target->sectorsPerTrack+Constants::FirstSector)};blocks.push_back(std::move(block));
    }
    return sparseSectorImage(*target,std::move(blocks));
}
}
