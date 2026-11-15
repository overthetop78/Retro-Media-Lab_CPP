#include "DiskCopy.h"
#include "Constants.h"
#include "../Apple/Geometry.h"
#include "../../Sources/AtomicImageWriter.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskCopy {
namespace {
using namespace Constants;
std::optional<DiagnosticCode> layoutError(const SectorImage& image){
    std::uint64_t available=0,dataLength=0;bool tagged=false,allTags=true;
    image.visitAvailableBlocks([&](auto logical){
        const auto block=image.block(logical);++available;
        if(!block)return false;
        if(dataLength>std::numeric_limits<std::uint64_t>::max()-block->data.size())throw MediaError(DiagnosticCode::DiskCopyRange);
        dataLength+=block->data.size();tagged|=block->tag.has_value();allTags&=block->tag&&block->tag->size()==TagSize;return true;
    });
    if(available!=image.blockCount())return DiagnosticCode::DiskCopyLayoutIncomplete;
    if(image.blockCount()>std::numeric_limits<std::uint64_t>::max()/DataBlockSize||dataLength!=image.blockCount()*DataBlockSize)return DiagnosticCode::DiskCopyLayoutSize;
    if(tagged&&!allTags)return DiagnosticCode::DiskCopyLayoutTags;
    return {};
}
void put(std::span<std::uint8_t> output,std::size_t offset,std::size_t size,std::uint32_t value){
    for(std::size_t i=0;i<size;++i)output[offset+i]=std::uint8_t(value>>((size-1-i)*BitsPerByte));
}
}
bool hasSupportedDataLayout(const SectorImage& image){return !layoutError(image);}
std::vector<std::uint8_t> build(const SectorImage& image,const Image& metadata){
    using namespace Constants;
    if(const auto error=layoutError(image))throw MediaError(*error);
    std::vector<std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;
    image.visitAvailableBlocks([&](auto logical){const auto block=image.block(logical);if(!block)throw MediaError(DiagnosticCode::DiskCopyLayoutIncomplete);blocks.push_back(block);return true;});
    std::sort(blocks.begin(),blocks.end(),[](const auto& left,const auto& right){return left->logicalBlock<right->logicalBlock;});
    std::vector<std::uint8_t> data,tags;
    for(const auto& block:blocks){
        data.insert(data.end(),block->data.begin(),block->data.end());
        if(block->tag)tags.insert(tags.end(),block->tag->begin(),block->tag->end());
    }
    if(data.size()>std::size_t(MaximumLength)-HeaderSize||tags.size()>std::size_t(MaximumLength)-HeaderSize-data.size())throw MediaError(DiagnosticCode::DiskCopyRange);
    std::vector<std::uint8_t> container(HeaderSize+data.size()+tags.size());
    const auto length=std::min(metadata.name.size(),MaximumNameLength);container[NameLengthOffset]=std::uint8_t(length);
    std::copy_n(metadata.name.begin(),length,container.begin()+NameOffset);
    put(container,DataLengthOffset,sizeof(std::uint32_t),std::uint32_t(data.size()));put(container,TagLengthOffset,sizeof(std::uint32_t),std::uint32_t(tags.size()));
    put(container,DataChecksumOffset,sizeof(std::uint32_t),checksum(data));
    put(container,TagChecksumOffset,sizeof(std::uint32_t),tags.empty()?MissingChecksum:checksum(Bytes(tags).subspan(TagChecksumExcludedPrefixSize)));
    container[DiskFormatOffset]=metadata.diskFormat;container[FormatByteOffset]=metadata.formatByte;put(container,PrivateWordOffset,sizeof(PrivateWord),PrivateWord);
    std::copy(data.begin(),data.end(),container.begin()+HeaderSize);std::copy(tags.begin(),tags.end(),container.begin()+HeaderSize+data.size());return container;
}
Image createMetadata(std::shared_ptr<const SectorImage> image,std::string_view name){
    using namespace Constants;namespace G=Apple::Geometry;
    if(!image)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    std::uint8_t diskFormat;
    if(image->capacity()==G::MacintoshGcr::Capacity400K)diskFormat=DiskFormat400K;
    else if(image->capacity()==G::MacintoshGcr::Capacity800K)diskFormat=DiskFormat800K;
    else if(image->capacity()==G::MacintoshMfm::Capacity)diskFormat=DiskFormat1440K;
    else throw MediaError(DiagnosticCode::RawUnknownGeometry,{std::to_string(image->capacity())});
    const auto& id=image->geometry().formatId;
    auto formatByte=FormatHfs;
    if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,LisaPrefix))formatByte=FormatLisa;
    else for(std::size_t offset=0;offset+MfsMarker.size()<=id.size();++offset)if(Primitives::asciiIdentifierEqualsIgnoreCase(std::string_view(id).substr(offset,MfsMarker.size()),MfsMarker)){formatByte=FormatMfs;break;}
    const auto text=Primitives::utf8Identifier(name);std::vector<std::uint8_t> bytes;bytes.reserve(std::size_t(text.size()));
    for(const auto value:text)bytes.push_back(value.unicode()<=AsciiMaximum?std::uint8_t(value.unicode()):std::uint8_t(AsciiReplacement));
    return {std::move(image),std::move(bytes),diskFormat,formatByte,{},{}};
}
void write(const SectorImage& image,const std::filesystem::path& path,const Image& metadata,std::stop_token token){
    const auto container=build(image,metadata);
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(container);},token);
}
void write(std::shared_ptr<const SectorImage> image,const std::filesystem::path& path,std::stop_token token){
    const auto stem=path.stem().u8string();const auto metadata=createMetadata(image,{reinterpret_cast<const char*>(stem.data()),stem.size()});
    write(*image,path,metadata,token);
}
}
