#include "Internal.h"
#include "../../../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaFileSystems::ProDos {
using namespace Constants;
bool canRead(const SectorImage& image,std::stop_token token){return Internal::header(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    const auto header=Internal::header(image,token);
    if(!header){const auto root=image.block(RootBlock,token);throw MediaError(DiagnosticCode::ProDosUnsupported,{std::to_string(RootBlock),Internal::hexByte(root&&root->data.size()>HeaderOffset?root->data[HeaderOffset]:std::uint8_t(0))});}
    FileSystemVolume volume;volume.name=header->name;volume.created=header->created;
    std::set<std::int32_t> visited;auto directory=Internal::directory(image,RootBlock,volume.warnings,visited,0,token);
    auto total=header->totalBlocks;
    if(std::uint64_t(total)>image.blockCount()){volume.warnings.push_back({DiagnosticCode::ProDosTotal,{std::to_string(total),std::to_string(image.blockCount())}});total=std::int32_t(image.blockCount());}
    const auto bitmap=Internal::bitmap(image,header->bitmapBlock,total,volume.warnings,token);
    volume.freeBytes=bitmap.valid?std::uint64_t(bitmap.freeBlocks)*BlockSize:0;
    volume.fileSystemId=Internal::sameId(image.geometry().formatId,Sos)?SosId:Id;volume.fileSystemDisplayName=volume.fileSystemId;
    volume.capacity=std::uint64_t(total)*BlockSize;volume.entries=std::move(directory.entries);return volume;
}
}
