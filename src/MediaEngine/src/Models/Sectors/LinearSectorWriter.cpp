#include "LinearSectorWriter.h"
#include "SectorConstants.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace SectorConstants;
using Geometry=MediaFileSystems::SectorGeometry;
bool same(std::string_view a,std::string_view b) {
    if(a.size()!=b.size())return false;
    const auto lower=[](char c){return c>=UpperFirst && c<=UpperLast?char(c+CaseDelta):c;};
    for(std::size_t index=0;index<a.size();++index)if(lower(a[index])!=lower(b[index]))return false;
    return true;
}
std::string describe(const Geometry& geometry) {
    const std::string separator(GeometrySeparator);
    return std::to_string(geometry.blockSize)+separator+std::to_string(geometry.cylinders)+separator+
        std::to_string(geometry.heads)+separator+std::to_string(geometry.sectorsPerTrack);
}
}
bool matchesSectorGeometry(const MediaFileSystems::SectorImage& image,const Geometry& target,bool requireFormatId) {
    if(!target.blockSize || !target.cylinders || !target.heads || !target.sectorsPerTrack)return false;
    const auto tracks=std::uint64_t(target.cylinders)*target.heads;
    if(tracks>std::numeric_limits<std::uint64_t>::max()/target.sectorsPerTrack)return false;
    const auto& source=image.geometry();
    return (!requireFormatId || same(source.formatId,target.formatId)) && source.blockSize==target.blockSize &&
        source.cylinders==target.cylinders && source.heads==target.heads && source.sectorsPerTrack==target.sectorsPerTrack &&
        image.blockCount()==tracks*target.sectorsPerTrack;
}
bool hasCompleteSectorPayload(const MediaFileSystems::SectorImage& image,std::uint32_t size) {
    std::uint64_t count=0;bool valid=true;
    image.visitAvailableBlocks([&](auto logical){
        const auto block=image.block(logical);
        if(!block || block->data.size()!=size){valid=false;return false;}
        ++count;return true;
    });
    return valid && count==image.blockCount();
}
void writeLinearSectorImage(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,const Geometry& target,bool requireFormatId,std::stop_token token) {
    checkCancellation(token);
    if(!matchesSectorGeometry(image,target,requireFormatId))
        throw MediaError(DiagnosticCode::RawGeometryMismatch,{image.geometry().formatId,describe(image.geometry()),target.formatId,describe(target)});
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){
        for(std::uint64_t logical=0;logical<image.blockCount();++logical){
            checkCancellation(stop);
            const auto block=image.block(logical,stop);
            if(!block)throw MediaError(DiagnosticCode::RawSectorMissing,{std::to_string(logical)});
            if(block->data.size()!=target.blockSize)
                throw MediaError(DiagnosticCode::RawSectorSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(target.blockSize)});
            output(block->data);
        }
    },token);
}
}
