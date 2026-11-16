#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::PhysicalMediaIds {
inline constexpr std::string_view TapePcm="tape-pcm-s16le-mono-v1",FluxTrack="flux-track-le32-v1";
inline constexpr std::string_view Encoding="encoding",SampleRate="sampleRate",Cylinder="cylinder",Head="head",DriveCylinder="driveCylinder",DriveHead="driveHead",Revolution="revolution",RevolutionCount="revolutionCount",TickNanoseconds="tickNanoseconds",IndexTimeNanoseconds="indexTimeNanoseconds",DiskType="diskType",DeviceId="deviceId",BusType="busType",DriveUnit="driveUnit";
}
