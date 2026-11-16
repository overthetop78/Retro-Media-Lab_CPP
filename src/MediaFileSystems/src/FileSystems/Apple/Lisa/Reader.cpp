#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Lisa {
namespace {
bool format(const SectorImage& image){return Identifiers::utf8(image.geometry().formatId).compare(Identifiers::utf8(Constants::ImageId),Qt::CaseInsensitive)==0;}
}
bool canRead(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(!format(image))return false;bool found=false;
    image.visitAvailableBlocks([&](auto number){checkCancellation(token);const auto block=image.block(number,token);if(block){const auto tag=Internal::tag(*block);found=tag&&tag->fileId==Constants::MddfFileId;}return !found;},token);return found;
}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto pages=Internal::pages(image,token);
    const bool tagged=format(image)&&std::any_of(pages.begin(),pages.end(),[](const auto& page){return page.tag&&page.tag->fileId==Constants::MddfFileId;});
    if(!tagged)throw MediaError(DiagnosticCode::LisaMissingTaggedSystem,{std::to_string(pages.size())});
    const auto mddf=Internal::mddf(pages);auto catalog=Internal::catalog(pages,mddf.version,token);FileSystemVolume volume;volume.warnings=std::move(catalog.warnings);
    std::uint64_t freePages=0,taggedPages=0;
    for(const auto& page:pages){checkCancellation(token);if(!page.tag)volume.warnings.push_back({DiagnosticCode::LisaInvalidTag,{std::to_string(page.block->logicalBlock),std::to_string(page.block->tag?page.block->tag->size():0),std::to_string(Constants::TagLength)}});else{++taggedPages;if(page.tag->fileId==Constants::FreePageFileId||page.tag->fileId==Constants::AlternateFreePageFileId)++freePages;}}
    auto contents=Internal::contents(pages,volume.warnings,token);
    for(auto& [id,file]:contents){checkCancellation(token);const auto name=catalog.names.find(id);if(name==catalog.names.end())continue;FileSystemEntry entry;entry.name=name->second;entry.kind=EntryKind::File;entry.size=file.bytes.size();entry.comment=std::string(Constants::DefaultFilePrefix)+Internal::hex(id);entry.storageReference=std::int64_t(file.firstBlock);entry.metadataValid=file.valid;entry.content=std::move(file.bytes);entry.commentResourceKey=Constants::FileDescriptionKey;entry.commentTranslationDomain=Constants::TranslationDomain;entry.commentArguments={Internal::hex(id)};volume.entries.push_back(std::move(entry));}
    volume.name=mddf.volumeName;volume.fileSystemId=Constants::Id;applyFileSystemDisplayName(volume,image.geometry(),mddf.version);volume.capacity=image.capacity();volume.freeBytes=freePages*image.geometry().blockSize;
    if(taggedPages==0)volume.warnings.push_back({DiagnosticCode::LisaUnknownFreeSpace,{}});return volume;
}
}
