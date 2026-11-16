#pragma once
#include <string>
#include <string_view>
namespace RetroMediaLab::MediaEngine::TrackEncodingAttributeKeys {
inline constexpr std::string_view SectorsPerTrack="sectorsPerTrack",Format="format",TracksPerSide="tracksPerSide",FmPhysicalLayout="fmPhysicalLayout",MfmPhysicalLayout="mfmPhysicalLayout",RequireCompletePhysicalTrack="requireCompletePhysicalTrack",TagPrefix="tag";
inline std::string tag(int index){return std::string(TagPrefix)+std::to_string(index);}
}
