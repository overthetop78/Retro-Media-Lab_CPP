#pragma once
#include <string_view>
#include <array>
namespace RetroMediaLab::MediaEngine::AudioConversionConstants {
inline constexpr std::string_view Directory="lib/FFmpeg",WindowsExecutable="ffmpeg.exe",UnixExecutable="ffmpeg",TemporaryPattern=".rml-audio-XXXXXX.tmp";
inline constexpr std::string_view HideBanner="-hide_banner",LogLevel="-loglevel",ErrorLevel="error",NoStdin="-nostdin",NoStats="-nostats",Input="-i",Map="-map",FirstAudio="0:a:0",NoVideo="-vn",NoSubtitles="-sn",NoData="-dn",Codec="-c:a",Muxer="-f",Rate="-ar",Channels="-ac",BitRate="-b:a",Overwrite="-y";
struct Target {std::string_view codec,muxer;};
inline constexpr std::array<Target,7> Targets{{{"pcm_s16le","wav"},{"libmp3lame","mp3"},{"libvorbis","ogg"},{"flac","flac"},{"aac","adts"},{"aac","ipod"},{"wmav2","asf"}}};
inline constexpr int MinimumRate=8000,MaximumRate=384000,MaximumChannels=32,MaximumBitRate=1024000,StartTimeoutMs=10000,WaitIntervalMs=50,CopyChunk=65536;
inline constexpr unsigned CreateNoWindow=0x08000000;
}
