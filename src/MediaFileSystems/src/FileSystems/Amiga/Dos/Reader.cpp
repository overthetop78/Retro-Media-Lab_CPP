#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
#include <algorithm>
#include <bit>
namespace RetroMediaLab::MediaFileSystems::AmigaDos {
namespace Internal {
namespace {
std::optional<std::vector<std::uint8_t>> rootData(const SectorImage& image,std::int64_t number,std::stop_token token){
    if(number<=0||std::uint64_t(number)>=image.blockCount())return {};
    const auto block=image.block(std::uint64_t(number),token);if(!block||block->data.size()!=BlockSize)return {};
    if(integer(block->data,PrimaryTypeOffset)!=HeaderPrimaryType||integer(block->data,SecondaryTypeOffset)!=RootSecondaryType)return {};return block->data;
}
Root makeRoot(Variant variant,std::int64_t number,std::vector<std::uint8_t> data){
    auto size=std::clamp(integer(data,HashTableSizeOffset),0,RootHashTableEntryCount);if(size==0)size=RootHashTableEntryCount;return {variant,number,std::move(data),size};
}
}
std::optional<Root> root(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(image.geometry().blockSize!=BlockSize)return {};
    const auto boot=image.block(0,token);if(!boot||boot->data.size()<=DosVariantOffset)return {};
    const auto signedBoot=dosPrefix(boot->data)&&boot->data[DosVariantOffset]<=MaximumVariant;
    const auto variant=signedBoot?Variant(boot->data[DosVariantOffset]):Variant::Ofs;
    const auto conventional=std::int64_t(image.blockCount()/BootBlockCount);
    if(!signedBoot){
        auto data=rootData(image,conventional,token);if(!data||!validChecksum(*data))return {};
        bool empty=true;for(int i=0;i<RootHashTableEntryCount;++i)if(integer(*data,DataPointersOffset+i*WordSize)!=0){empty=false;break;}
        if(empty)return {};return makeRoot(variant,conventional,std::move(*data));
    }
    const auto declared=integer(boot->data,BootRootPointerOffset);
    if(auto data=rootData(image,declared,token))return makeRoot(variant,declared,std::move(*data));
    if(auto data=rootData(image,conventional,token))return makeRoot(variant,conventional,std::move(*data));return {};
}
}
bool canRead(const SectorImage& image,std::stop_token token){checkCancellation(token);return Internal::root(image,token).has_value()||Internal::recover(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    const auto boot=image.block(0,token);
    if(boot&&Internal::dosPrefix(boot->data)&&boot->data[DosVariantOffset]>MaximumVariant)throw MediaError(DiagnosticCode::AmigaUnsupportedVariant,{std::to_string(boot->data[DosVariantOffset])});
    auto root=Internal::root(image,token);
    if(!root){if(auto recovered=Internal::recover(image,token))return std::move(*recovered);
        if(boot&&Internal::dosPrefix(boot->data)){const auto declared=Internal::integer(boot->data,BootRootPointerOffset);throw MediaError(DiagnosticCode::AmigaInvalidRoot,{std::to_string(declared>0?std::uint64_t(declared):image.blockCount()/BootBlockCount)});}
        throw MediaError(DiagnosticCode::AmigaUnsupportedBoot);
    }
    FileSystemVolume result;std::set<std::int64_t> visited{root->number};
    if(!validChecksum(root->data))result.warnings.push_back({DiagnosticCode::AmigaRootChecksum,{std::to_string(root->number)}});
    if(const auto block=image.block(std::uint64_t(root->number),token);block&&!Internal::recordedIntegrity(*block))result.warnings.push_back({DiagnosticCode::AmigaRecordedIntegrity,{std::to_string(root->number)}});
    result.entries=Internal::directory(image,root->data,root->hashSize,root->variant,visited,result.warnings,0,token);
    std::uint64_t free=0;
    const auto countBitmap = [&](std::int32_t number) {
        checkCancellation(token);
        const auto block=number>=0&&std::uint64_t(number)<image.blockCount()?image.block(std::uint64_t(number),token):nullptr;
        if(!block){result.warnings.push_back({DiagnosticCode::AmigaMissingBitmap,{std::to_string(number)}});return;}
        if(!validChecksum(block->data))result.warnings.push_back({DiagnosticCode::AmigaBitmapChecksum,{std::to_string(number)}});
        if(block->data.size()!=BlockSize){result.warnings.push_back({DiagnosticCode::AmigaShortBlock,{std::to_string(number),std::to_string(block->data.size())}});return;}
        if(!Internal::recordedIntegrity(*block))result.warnings.push_back({DiagnosticCode::AmigaRecordedIntegrity,{std::to_string(number)}});
        for(int offset=BitmapDataOffset;offset<BlockSize;offset+=WordSize)free+=std::popcount(Internal::unsignedInteger(block->data,offset));
    };
    const auto countPointers = [&](Bytes data, int offset, int count) {
        for (int i = 0; i < count; ++i) {
            const auto number = Internal::integer(data, offset + i * WordSize);
            if (!number) break;
            if (visited.insert(number).second) countBitmap(number);
        }
    };
    countPointers(root->data, BitmapPointersOffset, MaximumBitmapPointerCount);
    auto extension = Internal::integer(root->data, BitmapExtensionOffset);
    constexpr auto BitmapBits = BitmapDataWordCount * WordBits;
    const auto maximumExtensions = image.blockCount() / (BitmapBits * BitmapDataWordCount) + 1;
    for (std::uint64_t count = 0; extension && count < maximumExtensions; ++count) {
        checkCancellation(token);
        if (extension < BootBlockCount || std::uint64_t(extension) >= image.blockCount() ||
            !visited.insert(extension).second) {
            result.warnings.push_back({DiagnosticCode::AmigaMissingBitmap, {std::to_string(extension)}});
            break;
        }
        const auto block = image.block(std::uint64_t(extension), token);
        if (!block || block->data.size() != BlockSize || !Internal::recordedIntegrity(*block)) {
            result.warnings.push_back({DiagnosticCode::AmigaMissingBitmap, {std::to_string(extension)}});
            break;
        }
        countPointers(block->data, 0, BitmapDataWordCount);
        extension = Internal::integer(block->data, BitmapExtensionNextOffset);
    }
    result.name=readName(root->data,OrdinaryNameOffset,OrdinaryNameMaximumLength);result.fileSystemId=fileSystemId(root->variant);applyFileSystemDisplayName(result,image.geometry());
    result.capacity=image.capacity();result.freeBytes=std::min(free,image.blockCount())*BlockSize;
    result.created=readTime(root->data,DateOffset);result.modified=readTime(root->data,VolumeModifiedDateOffset);return result;
}
}
