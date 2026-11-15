#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::SpartaDos {
using namespace Constants;
bool canRead(const SectorImage& image,std::stop_token token){return Internal::header(image,token).has_value();}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    const auto header=Internal::header(image,token);if(!header)throw MediaError(DiagnosticCode::SpartaUnsupported,{image.geometry().formatId});auto root=Internal::directory(image,header->root,{},token);FileSystemVolume volume;volume.name=header->name;volume.fileSystemId=Id;volume.fileSystemDisplayName=System;volume.capacity=std::uint64_t(header->total)*header->size;volume.freeBytes=std::uint64_t(header->free)*header->size;volume.entries=std::move(root.children);volume.attributes.push_back(std::string(VolumeAttribute));volume.attributes.push_back(std::string(header->version==Version1?Version1Attribute:Version2Attribute));if(header->writeProtected)volume.attributes.push_back(std::string(WriteProtectedAttribute));volume.bootable=header->bootMap!=0;return volume;
}
}
