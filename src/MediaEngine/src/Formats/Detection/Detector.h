#pragma once
#include <RetroMediaLab/MediaEngine/ImageFormatDetection.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <functional>
namespace RetroMediaLab::MediaEngine::Detection {
using Source=std::shared_ptr<const MediaFileSystems::RandomAccessData>;
using OpenRead=std::function<Source(const std::filesystem::path&)>;
struct Context {
 std::span<const DiskFormat> formats;std::filesystem::path path;std::optional<std::int64_t> length;std::string extension;std::vector<DiskFormat> candidates;OpenRead openRead;std::stop_token token;
 const DiskFormat* find(std::string_view)const;
 DetectedImageFormat result(std::optional<std::string_view>,FormatConfidence,std::string_view explanation)const;
 DetectedImageFormat ambiguous(std::string_view explanation)const;
 Source open()const;
 std::vector<std::uint8_t> read(const Source&,std::uint64_t offset,std::size_t count)const;
};
template<class Range>std::optional<std::string_view> sizeFormat(const Range& sizes,std::optional<std::int64_t> size){if(size)for(const auto& [length,id]:sizes)if(length==*size)return id;return {};}
bool probeFailure(const std::exception_ptr&);
std::optional<DetectedImageFormat> adf(const Context&);
std::optional<DetectedImageFormat> atari(const Context&);
std::optional<DetectedImageFormat> macintosh(const Context&);
RETROMEDIALAB_MEDIAENGINE_API DetectedImageFormat detect(std::span<const DiskFormat>,const std::filesystem::path&,std::optional<std::int64_t> knownLength={},OpenRead={},std::stop_token={});
}
