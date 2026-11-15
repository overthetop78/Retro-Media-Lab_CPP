#include "Constants.h"
#include "../FormatRecognition.h"
#include "../../../Primitives/Binary.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AtariMetadata {
using namespace KFileConstants;
bool kFile(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(!supported(image.geometry().formatId)||image.geometry().blockSize!=SectorSize||image.blockCount()<=BootSectorCount)return false;
    std::shared_ptr<const SectorBlock> header;
    for(std::size_t index=0;index<BootSectorCount;++index){const auto block=image.block(index,token);if(!block||block->data.size()!=SectorSize)return false;if(index==0)header=block;}
    const Bytes h=header->data;if(h[FlagOffset]!=0||h[CountOffset]!=BootSectorCount||Binary::little<std::uint16_t>(h,LoadOffset)!=BootLoad||h[JumpOffset]!=Jump||h[JumpLowOffset]!=JumpLow||h[JumpHighOffset]!=JumpHigh)return false;
    const auto length=std::size_t(h[PayloadOffset])|(std::size_t(h[PayloadOffset+1])<<MiddleShift)|(std::size_t(h[PayloadOffset+MinimumExecutableLength])<<HighShift);
    if(length==0||length>(image.blockCount()-BootSectorCount)*image.geometry().blockSize)return false;
    std::vector<std::uint8_t> payload(length);std::size_t written=0;
    for(std::uint64_t logical=BootSectorCount;written<length;++logical){checkCancellation(token);if(logical>=image.blockCount())return false;const auto block=image.block(logical,token);if(!block)return false;const auto count=std::min(block->data.size(),length-written);std::copy_n(block->data.begin(),count,payload.begin()+std::ptrdiff_t(written));written+=count;}
    return payload.size()>=MinimumExecutableLength&&Binary::little<std::uint16_t>(payload,0)==Executable;
}
FileSystemVolume readKFile(const SectorImage& image,std::stop_token token){if(!kFile(image,token))throw MediaError(DiagnosticCode::AtariKFileUnsupported);FileSystemVolume volume;volume.fileSystemId=Id;volume.fileSystemDisplayName=Display;volume.capacity=image.capacity();volume.freeSpaceKnown=false;volume.attributes={std::string(Bootable),std::string(SingleProgram),std::string(KBoot)};volume.bootable=true;return volume;}
}
