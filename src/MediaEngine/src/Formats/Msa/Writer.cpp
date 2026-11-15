#include "Msa.h"
#include "Constants.h"
#include "../St/Geometry.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Msa {
namespace {
using namespace Constants;
void word(std::vector<std::uint8_t>& bytes,std::size_t offset,std::uint32_t value){bytes[offset]=std::uint8_t(value>>BitsPerByte);bytes[offset+1]=std::uint8_t(value);}
MediaFileSystems::SectorGeometry validate(const MediaFileSystems::SectorImage& image){const auto& g=image.geometry();const auto known=St::geometry(g.formatId);if(!known||g.blockSize!=SectorSize||g.cylinders!=known->cylinders||g.heads!=known->heads||g.sectorsPerTrack!=known->sectorsPerTrack)throw MediaError(DiagnosticCode::MsaUnsupportedImage,{g.formatId,std::to_string(g.cylinders),std::to_string(g.heads),std::to_string(g.sectorsPerTrack),std::to_string(g.blockSize)});return *known;}
std::vector<std::uint8_t> track(const MediaFileSystems::SectorImage& image,const MediaFileSystems::SectorGeometry& g,std::uint32_t cylinder,std::uint32_t head){std::vector<std::uint8_t> result(g.sectorsPerTrack*SectorSize);for(std::uint32_t sector=0;sector<g.sectorsPerTrack;++sector){const auto logical=(cylinder*g.heads+head)*g.sectorsPerTrack+sector;const auto block=image.block(logical);if(!block)throw MediaError(DiagnosticCode::MsaMissingSector,{std::to_string(logical),std::to_string(cylinder),std::to_string(head),std::to_string(sector+FirstSector)});if(block->data.size()!=SectorSize)throw MediaError(DiagnosticCode::MsaSectorSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(SectorSize)});std::copy(block->data.begin(),block->data.end(),result.begin()+sector*SectorSize);}return result;}
void emitData(const MediaFileSystems::SectorImage& image,const MediaFileSystems::SectorGeometry& g,const ImageOutput& output,std::stop_token token){std::vector<std::uint8_t> header(HeaderSize);word(header,SignatureOffset,Signature);word(header,SectorsOffset,g.sectorsPerTrack);word(header,HeadsOffset,g.heads-1);word(header,StartOffset,0);word(header,EndOffset,g.cylinders-1);output(header);for(std::uint32_t cylinder=0;cylinder<g.cylinders;++cylinder)for(std::uint32_t head=0;head<g.heads;++head){checkCancellation(token);const auto raw=track(image,g,cylinder,head),packed=pack(raw);const auto& bytes=packed.size()<raw.size()?packed:raw;std::vector<std::uint8_t> length(WordSize);word(length,0,std::uint32_t(bytes.size()));output(length);output(bytes);}}
}
std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage& image,std::stop_token token){const auto geometry=validate(image);std::vector<std::uint8_t> bytes;emitData(image,geometry,[&](auto part){bytes.insert(bytes.end(),part.begin(),part.end());},token);return bytes;}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token){const auto geometry=validate(image);writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){emitData(image,geometry,output,stop);},token);}
}
