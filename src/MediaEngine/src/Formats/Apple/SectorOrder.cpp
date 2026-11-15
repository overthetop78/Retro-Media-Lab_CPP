#include "SectorOrder.h"
#include "Geometry.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apple {
using namespace Geometry::AppleII;
namespace {
void validateSector(std::int32_t sector){if(sector<0||sector>=SectorsPerTrack)throw MediaError(DiagnosticCode::AppleOrderSector,{std::to_string(sector),std::to_string(SectorsPerTrack-1)});}
}
std::int32_t proDosToPhysicalSector(std::int32_t sector){validateSector(sector);return ProDosToPhysical[sector];}
std::int32_t physicalToDosFileSector(std::int32_t sector){validateSector(sector);return PhysicalToDos[sector];}
std::vector<std::uint8_t> dosToProDos(MediaFileSystems::Bytes data){
    if(data.size()%TrackSize)throw MediaError(DiagnosticCode::RawInvalidLength,{std::to_string(data.size()),std::to_string(TrackSize)});
    std::vector<std::uint8_t> output(data.size());
    for(std::size_t track=0;track<data.size()/TrackSize;++track)
        for(std::int32_t sector=0;sector<SectorsPerTrack;++sector){
            const auto source=(track*SectorsPerTrack+physicalToDosFileSector(proDosToPhysicalSector(sector)))*SectorSize;
            const auto destination=(track*SectorsPerTrack+sector)*SectorSize;
            std::copy_n(data.begin()+source,SectorSize,output.begin()+destination);
        }
    return output;
}
std::vector<std::uint8_t> proDosToDos(MediaFileSystems::Bytes data){return dosToProDos(data);}
}
