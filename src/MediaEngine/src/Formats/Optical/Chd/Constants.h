#pragma once
#include "../../Common/Chd/ContainerConstants.h"
#include "../../../Models/Optical/Track.h"
#include <array>
namespace RetroMediaLab::MediaEngine::ChdOptical::Constants {
using namespace Chd::Constants;
inline constexpr std::string_view FormatId="optical-chd",Extension=".chd",NoCompression="none",VersionKey="version",CompressionKey="compression",HunkSizeKey="hunkSize",UnitSizeKey="unitSize",TrackCountKey="trackCount",ProfileKey="mediaProfile",CdProfile="CD",DvdProfile="DVD";
inline constexpr std::uint64_t MaximumSigned=0x7fffffffffffffff,MaximumBuffer=0x7fffffff;
inline constexpr std::uint32_t TrackTag=0x43485452,Track2Tag=0x43485432,DvdTag=0x44564420;
inline constexpr std::string_view TrackPrefix="TRACK:",TrackKey="TRACK",TypeKey="TYPE",SubtypeKey="SUBTYPE",FramesKey="FRAMES",PregapKey="PREGAP",PgtypeKey="PGTYPE",PgsubKey="PGSUB",PostgapKey="POSTGAP",NoSubchannel="NONE",RwSubchannel="RW",RawSubchannel="RW_RAW";
inline constexpr std::array<std::string_view,8> RequiredKeys{TrackKey,TypeKey,SubtypeKey,FramesKey,PregapKey,PgtypeKey,PgsubKey,PostgapKey};
inline constexpr char TokenSeparator=' ',ValueSeparator=':',StoredPregapPrefix='V';
inline constexpr int SimpleKeys=4,TrackAlignment=4,Data2048=2048,Data2336=2336,Data2324=2324,Raw2352=2352,Subchannel96=96,RawUserOffset=16;
struct TrackType {std::string_view name;OpticalTrackMode mode;int size;};
inline constexpr std::array<TrackType,15> TrackTypes{{
 {"MODE1",OpticalTrackMode::Mode1Data2048,Data2048},{"MODE1/2048",OpticalTrackMode::Mode1Data2048,Data2048},{"MODE1_RAW",OpticalTrackMode::Mode1Raw2352,Raw2352},{"MODE1/2352",OpticalTrackMode::Mode1Raw2352,Raw2352},
 {"MODE2",OpticalTrackMode::Mode2Data2336,Data2336},{"MODE2/2336",OpticalTrackMode::Mode2Data2336,Data2336},{"MODE2_FORM1",OpticalTrackMode::Mode2Form1Data2048,Data2048},{"MODE2/2048",OpticalTrackMode::Mode2Form1Data2048,Data2048},{"MODE2_FORM2",OpticalTrackMode::Mode2Form2Data2324,Data2324},{"MODE2/2324",OpticalTrackMode::Mode2Form2Data2324,Data2324},
 {"MODE2_FORM_MIX",OpticalTrackMode::Mode2FormMixData2336,Data2336},{"MODE2_RAW",OpticalTrackMode::Mode2Raw2352,Raw2352},{"MODE2/2352",OpticalTrackMode::Mode2Raw2352,Raw2352},{"CDI/2352",OpticalTrackMode::Mode2Raw2352,Raw2352},{"AUDIO",OpticalTrackMode::Audio,Raw2352}}};
}
