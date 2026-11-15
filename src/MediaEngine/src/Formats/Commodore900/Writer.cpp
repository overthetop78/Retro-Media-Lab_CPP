#include "CoherentImages.h"
#include "Constants.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Commodore900 {
using namespace Constants;
bool canWrite(const MediaFileSystems::SectorImage& image){const auto& g=image.geometry();return Internal::sameFormat(g.formatId)&&g.blockSize==SectorSize&&g.cylinders==CylinderCount&&g.heads==HeadCount&&g.sectorsPerTrack==MaximumSectors&&image.blockCount()<=BlockCount;}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token){
 checkCancellation(token);if(!canWrite(image))throw MediaError(DiagnosticCode::C900WriterGeometry);
 writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){
  for(std::uint64_t logical=0;logical<image.blockCount();++logical){
   checkCancellation(stop);const auto block=image.block(logical,stop);
   if(!block)throw MediaError(DiagnosticCode::RawSectorMissing,{std::to_string(logical)});
   if(block->address!=addressOf(std::int64_t(logical)))throw MediaError(DiagnosticCode::C900BlockAddress,{std::to_string(logical)});
   if(block->data.size()!=SectorSize)throw MediaError(DiagnosticCode::RawSectorSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(SectorSize)});
   output(block->data);
  }
 },token);
}
}
