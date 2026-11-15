#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive {
namespace {
std::string blocks(const std::vector<std::uint64_t>& values){std::string text;for(const auto value:values){if(!text.empty())text+=Constants::BlockSeparator;text+=std::to_string(value);}return text;}
}
bool canRead(const SectorImage& image,std::stop_token token){return Internal::directory(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    const auto descriptors=Internal::directory(image,token);if(!descriptors)throw MediaError(DiagnosticCode::AmigaFlatInvalidDirectory);
    FileSystemVolume volume;volume.fileSystemId=Constants::FileSystemId;volume.fileSystemDisplayName=Constants::FileSystemId;volume.capacity=image.capacity();auto offset=std::uint64_t(descriptors->front().length);
    for(std::size_t index=1;index<descriptors->size();++index){
        checkCancellation(token);const auto& descriptor=(*descriptors)[index];auto result=Internal::data(image,offset,descriptor.length,token);
        if(!result.missing.empty())volume.warnings.push_back({DiagnosticCode::AmigaFlatMissingBlocks,{descriptor.name,blocks(result.missing)}});
        if(!result.invalid.empty())volume.warnings.push_back({DiagnosticCode::AmigaFlatInvalidBlocks,{descriptor.name,blocks(result.invalid)}});
        FileSystemEntry entry;entry.name=descriptor.name;entry.kind=EntryKind::File;entry.size=descriptor.length;entry.comment=Constants::EntryComment;
        entry.commentResourceKey=Constants::CommentKey;entry.commentTranslationDomain=Constants::TranslationDomain;
        entry.storageReference=std::int64_t(offset/image.geometry().blockSize);entry.content=std::move(result.bytes);volume.entries.push_back(std::move(entry));offset+=descriptor.length;
    }
    volume.freeBytes=volume.capacity-offset;return volume;
}
}
