#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Adfs {
namespace Internal {
bool validIntegrity(const SectorBlock& block) noexcept{return block.integrityValid!=false&&block.headerIntegrityValid!=false&&block.dataIntegrityValid!=false;}
std::unique_ptr<FileCore::AddressResolver> layout(const SectorImage& image,std::stop_token token,std::vector<std::uint64_t>* mapBlocks){
    using namespace Constants;checkCancellation(token);
    if(image.geometry().blockSize==UnitSize){
        const auto first=image.block(0,token),second=image.block(1,token);if(!first||!second)return {};
        std::vector<std::uint8_t> map=first->data;map.insert(map.end(),second->data.begin(),second->data.end());
        const auto interleaved=image.geometry().heads==InterleavedHeads&&image.geometry().sectorsPerTrack==InterleavedSectors;
        if(mapBlocks)*mapBlocks={0,1};
        return FileCore::oldMap(map,image.capacity(),{BbcRootAddress,interleaved?std::int32_t(image.geometry().cylinders):0,interleaved?std::int32_t(image.geometry().heads):1,interleaved?std::int32_t(image.geometry().sectorsPerTrack):0});
    }
    if(image.geometry().blockSize!=BlockSize)return {};
    if(auto map=FileCore::newMap(image,token))return map;
    const auto first=image.block(0,token);if(!first)return {};if(mapBlocks)mapBlocks->push_back(0);return FileCore::oldMap(first->data,image.capacity());
}
}
bool canRead(const SectorImage& image,std::stop_token token){
    checkCancellation(token);auto resolver=Internal::layout(image,token);if(!resolver)return false;
    bool integrity=true;return Internal::directoryBytes(image,*resolver,resolver->rootAddress(),integrity,nullptr,token).has_value();
}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    std::vector<std::uint64_t> mapBlocks;auto resolver=Internal::layout(image,token,&mapBlocks);
    if(!resolver)throw MediaError(DiagnosticCode::AdfsUnsupportedImage,{std::to_string(image.geometry().blockSize),std::to_string(image.blockCount())});
    Internal::Context context{image,std::move(resolver),{},true};
    bool rootIntegrity=true;
    if(!Internal::directoryBytes(image,*context.resolver,context.resolver->rootAddress(),rootIntegrity,nullptr,token))throw MediaError(DiagnosticCode::AdfsUnsupportedImage,{std::to_string(image.geometry().blockSize),std::to_string(image.blockCount())});
    for(const auto i:mapBlocks)if(const auto block=image.block(i,token);block&&!Internal::validIntegrity(*block)){context.mapIntegrity=false;context.warnings.push_back({DiagnosticCode::AdfsIntegrity,{std::to_string(i)}});}
    std::set<std::int32_t> visited;auto root=Internal::directory(context,context.resolver->rootAddress(),visited,0,context.mapIntegrity,token);
    FileSystemVolume result;result.name=context.resolver->volumeName().empty()?root.name:context.resolver->volumeName();
    result.fileSystemId=Constants::FileSystemId;applyFileSystemDisplayName(result,image.geometry());
    result.capacity=image.capacity();result.freeBytes=context.resolver->freeBytes();result.freeSpaceKnown=context.mapIntegrity&&root.integrity;
    result.entries=std::move(root.entries);result.warnings=std::move(context.warnings);return result;
}
}
