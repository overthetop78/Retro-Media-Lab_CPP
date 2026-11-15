#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaEngine::OpticalConstants {
inline constexpr std::int32_t MaximumIndex=99,CompatibilitySectorSize=2048;
inline constexpr std::string_view Layers="layerCount",Faces="faceCount";
inline constexpr std::string_view Session="sessionNumber",Track="trackNumber",FirstSector="firstSector",SectorCount="sectorCount",StoredSectorSize="storedSectorSize",UserOffset="userDataOffset",UserLength="userDataLength",SourceOffset="sourceOffset",Pregap="pregapSectors",Postgap="postgapSectors",SubchannelOffset="subchannelSourceOffset",SubchannelBytes="subchannelBytesPerSector",StoredPregap="storedPregapSectors",SubchannelStride="subchannelStride";
}
