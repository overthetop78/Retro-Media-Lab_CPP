#include "Raw.h"
#include "Geometry.h"
#include "SectorOrder.h"
#include "Constants.h"
#include "../FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
using Image=MediaFileSystems::SectorImage;
using Primitives::asciiIdentifierEqualsIgnoreCase;
bool same(std::string_view a,std::string_view b) noexcept{return asciiIdentifierEqualsIgnoreCase(a,b);}
std::string dimensions(std::uint64_t size,std::uint64_t count,std::uint64_t capacity){return std::to_string(size)+std::string(Constants::GeometrySeparator)+std::to_string(count)+std::string(Constants::GeometrySeparator)+std::to_string(capacity);}
[[noreturn]] void badGeometry(const Image& image,std::string_view target,std::uint64_t size,std::uint64_t count,std::uint64_t capacity){
    throw MediaError(DiagnosticCode::RawGeometryMismatch,{image.geometry().formatId,dimensions(image.geometry().blockSize,image.blockCount(),image.capacity()),std::string(target),dimensions(size,count,capacity)});
}
std::shared_ptr<const MediaFileSystems::SectorBlock> blockAt(const Image& image,std::uint64_t logical){
    const auto block=image.block(logical);
    if(!block)throw MediaError(DiagnosticCode::RawSectorMissing,{std::to_string(logical)});
    return block;
}
void copyBlock(const MediaFileSystems::SectorBlock& block,std::uint32_t size,std::span<std::uint8_t> output,std::uint64_t logical){
    if(block.data.size()!=size)throw MediaError(DiagnosticCode::RawSectorSize,{std::to_string(logical),std::to_string(block.data.size()),std::to_string(size)});
    std::copy(block.data.begin(),block.data.end(),output.begin()+logical*size);
}
std::vector<std::uint8_t> linear(const Image& image,std::string_view target,std::uint32_t size,std::uint32_t count,std::uint32_t capacity){
    if(image.geometry().blockSize!=size||image.blockCount()!=count||image.capacity()!=capacity)badGeometry(image,target,size,count,capacity);
    std::vector<std::uint8_t> output(capacity);
    for(std::uint64_t logical=0;logical<count;++logical)copyBlock(*blockAt(image,logical),size,output,logical);
    return output;
}
std::uint32_t macintoshCount(const Image& image) noexcept{
    if(image.capacity()==Geometry::MacintoshGcr::Capacity400K)return Geometry::MacintoshGcr::SingleSidedBlockCount;
    if(image.capacity()==Geometry::MacintoshGcr::Capacity800K)return Geometry::MacintoshGcr::SingleSidedBlockCount*Geometry::DoubleHeads;
    if(image.capacity()==Geometry::MacintoshMfm::Capacity)return Geometry::MacintoshMfm::SectorCount;
    return 0;
}
[[noreturn]] void unsupported(std::string_view target,std::string_view extension){throw MediaError(DiagnosticCode::UnsupportedRawFormat,{std::string(target)+std::string(Constants::TargetSeparator)+std::string(extension)});}
}
bool isProDos140(std::string_view id) noexcept{return same(id,Formats::AppleIIProDos140)||same(id,Formats::AppleIIISos);}
std::vector<std::uint8_t> twoImgPayload(const Image& image,std::string_view target){
    using namespace Geometry;
    if(same(target,Formats::AppleIIAppleDos113))return linear(image,target,AppleII::SectorSize,AppleII::TrackCount*AppleII::Dos32SectorsPerTrack,AppleII::Dos32Capacity);
    if(same(target,Formats::AppleIIAppleDos140))return linear(image,target,AppleII::SectorSize,AppleII::TrackCount*AppleII::SectorsPerTrack,AppleII::Capacity);
    if(isProDos140(target))return linear(image,target,AppleII::ProDosBlockSize,AppleII::TrackCount*AppleII::ProDosBlocksPerTrack,AppleII::Capacity);
    if(same(target,Formats::AppleIIProDos800))return linear(image,target,MacintoshGcr::BlockSize,MacintoshGcr::SingleSidedBlockCount*DoubleHeads,MacintoshGcr::Capacity800K);
    unsupported(target,Constants::TwoMg);
}
std::vector<std::uint8_t> rawPayload(const Image& image,std::string_view target,std::string_view extension){
    if(same(target,Formats::AppleIIAppleDos113)&&same(extension,Constants::D13))return twoImgPayload(image,target);
    const bool dosOrder=same(extension,Constants::Do)||same(extension,Constants::Dsk);
    if(same(target,Formats::AppleIIAppleDos140)&&dosOrder)return twoImgPayload(image,target);
    if(isProDos140(target)){
        if(same(extension,Constants::Po))return twoImgPayload(image,target);
        if(dosOrder)return proDosToDos(twoImgPayload(image,target));
    }
    if(same(target,Formats::AppleIIProDos800)&&same(extension,Constants::Po))return twoImgPayload(image,target);
    unsupported(target,extension);
}
bool hasMacintoshGeometry(const Image& image) noexcept{
    const auto count=macintoshCount(image);
    return count>0&&image.geometry().blockSize==Geometry::MacintoshGcr::BlockSize&&image.blockCount()==count;
}
std::vector<std::uint8_t> macintoshPayload(const Image& image){
    const auto count=macintoshCount(image);
    if(!count)throw MediaError(DiagnosticCode::RawUnknownGeometry,{std::to_string(image.capacity())});
    if(!hasMacintoshGeometry(image))badGeometry(image,Formats::AppleMacPrefix,Geometry::MacintoshGcr::BlockSize,count,image.capacity());
    std::vector<std::uint8_t> output(std::size_t(count)*Geometry::MacintoshGcr::BlockSize);
    for(std::uint32_t logical=0;logical<count;++logical){
        const auto block=blockAt(image,logical);
        MediaFileSystems::SectorAddress address;
        if(image.capacity()==Geometry::MacintoshMfm::Capacity){
            using namespace Geometry::MacintoshMfm;
            address={logical/(HeadCount*SectorsPerTrack),logical/SectorsPerTrack%HeadCount,logical%SectorsPerTrack};
        }else address=Geometry::MacintoshGcr::address(std::int32_t(logical),std::int32_t(image.geometry().heads));
        if(block->address!=address)throw MediaError(DiagnosticCode::AppleWriteAddress,{std::to_string(logical)});
        copyBlock(*block,Geometry::MacintoshGcr::BlockSize,output,logical);
    }
    return output;
}
void writeRaw(const Image& image,const std::filesystem::path& path,std::string_view target,std::stop_token token){
    const auto payload=rawPayload(image,target,path.extension().string());
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(payload);},token);
}
void writeMacintosh(const Image& image,const std::filesystem::path& path,std::stop_token token){
    const auto payload=macintoshPayload(image);
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(payload);},token);
}
}
