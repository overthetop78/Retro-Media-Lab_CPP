#include "Rx02.h"
#include "SectorOrder.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Rx02 {
bool canWrite(const MediaFileSystems::SectorImage& image){using namespace Constants;const auto& g=image.geometry();return Primitives::ordinalIdentifierEqualsIgnoreCase(g.formatId,FormatId)&&g.blockSize==LogicalSize&&image.blockCount()==LogicalCount&&g.cylinders==Tracks&&g.heads==Heads;}
std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage& image,std::stop_token token){using namespace Constants;if(!canWrite(image))throw MediaError(DiagnosticCode::Rx02WriteGeometry);std::vector<std::uint8_t> bytes(Capacity);
    for(std::uint32_t logical=0;logical<LogicalCount;++logical){MediaFileSystems::checkCancellation(token);const auto block=image.block(logical,token);if(!block)throw MediaError(DiagnosticCode::RawSectorMissing,{std::to_string(logical)});if(block->data.size()!=LogicalSize)throw MediaError(DiagnosticCode::RawSectorSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(LogicalSize)});for(std::uint32_t part=0;part<Parts;++part)std::copy_n(block->data.begin()+std::ptrdiff_t(part*PhysicalSize),PhysicalSize,bytes.begin()+std::ptrdiff_t(physicalOffset(logical*Parts+part)));}
    return bytes;
}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(image,token);writeAtomicImage(path,[&](const ImageOutput& out,std::stop_token stop){MediaFileSystems::checkCancellation(stop);out(bytes);},token);}
}
