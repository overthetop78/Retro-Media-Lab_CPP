#include <RetroMediaLab/MediaFileSystems/SectorReader.h>
namespace RetroMediaLab::MediaFileSystems {
bool SectorReader::canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    checkCancellation(token);const auto* sectors=dynamic_cast<const SectorRepresentation*>(&document.representation());
    return sectors&&volume.start()==0&&std::uint64_t(volume.length())==sectors->image().capacity()&&canRead(sectors->image(),token);
}
FileSystemVolume SectorReader::read(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    checkCancellation(token);const auto* sectors=dynamic_cast<const SectorRepresentation*>(&document.representation());
    if(!sectors||volume.start()!=0||std::uint64_t(volume.length())!=sectors->image().capacity())throw MediaError(DiagnosticCode::WholeVolumeRequired);
    return read(sectors->image(),token);
}
}
