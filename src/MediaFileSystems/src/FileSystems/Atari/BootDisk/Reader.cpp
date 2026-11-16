#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Constants.h"
#include "../FormatRecognition.h"
#include <RetroMediaLab/MediaFileSystems/AtariDos.h>
#include "../../../Primitives/Binary.h"
namespace RetroMediaLab::MediaFileSystems::AtariMetadata {
using namespace BootConstants;
bool bootDisk(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(!supported(image.geometry().formatId)||image.blockCount()==0)return false;const auto first=image.block(0,token);if(!first||first->data.size()!=SectorSize||first->data.size()<HeaderLength)return false;
    const auto count=first->data[CountOffset];if(count==0||count>image.blockCount()||std::size_t(Binary::little<std::uint16_t>(first->data,LoadOffset))+std::size_t(count)*SectorSize>AddressSpace)return false;
    for(std::uint64_t index=0;index<count;++index){const auto sector=image.block(index,token);if(!sector||sector->data.size()<SectorSize)return false;}
    return !AtariDos::canRead(image,token)&&!kFile(image,token)&&!clk(image,token);
}
FileSystemVolume readBootDisk(const SectorImage& image,std::stop_token token){if(!bootDisk(image,token))throw MediaError(DiagnosticCode::AtariBootUnsupported);FileSystemVolume volume;volume.fileSystemId=Id;applyFileSystemDisplayName(volume,image.geometry());volume.capacity=image.capacity();volume.freeSpaceKnown=false;volume.attributes={std::string(Bootable),std::string(Direct)};volume.bootable=true;return volume;}
}
