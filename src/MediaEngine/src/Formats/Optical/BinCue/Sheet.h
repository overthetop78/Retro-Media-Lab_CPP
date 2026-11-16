#pragma once
#include "Constants.h"
#include <filesystem>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::BinCue {
enum class FileKind {Binary,WavePcm,Vorbis};
struct TrackFields {int number=0;OpticalTrackMode mode=OpticalTrackMode::Audio;std::vector<OpticalTrackIndex> indexes;std::int64_t pregap=0,postgap=0;std::vector<std::string> flags;std::optional<std::string> isrc;OpticalMetadata metadata;};
class TrackDeclaration final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit TrackDeclaration(TrackFields);
    const TrackFields& fields()const noexcept{return fields_;}
private:TrackFields fields_;
};
class FileDeclaration final {
public:
    RETROMEDIALAB_MEDIAENGINE_API FileDeclaration(std::string,FileKind,std::vector<TrackDeclaration>);
    const std::string& path()const noexcept{return path_;}FileKind kind()const noexcept{return kind_;}std::span<const TrackDeclaration> tracks()const noexcept{return tracks_;}
private:std::string path_;FileKind kind_;std::vector<TrackDeclaration> tracks_;
};
class Sheet final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Sheet(std::vector<FileDeclaration>,std::optional<std::string> catalog={},OpticalMetadata metadata={});
    std::span<const FileDeclaration> files()const noexcept{return files_;}const std::optional<std::string>& catalog()const noexcept{return catalog_;}const OpticalMetadata& metadata()const noexcept{return metadata_;}
private:std::vector<FileDeclaration> files_;std::optional<std::string> catalog_;OpticalMetadata metadata_;
};
RETROMEDIALAB_MEDIAENGINE_API Sheet parseSheet(std::span<const std::string>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Sheet readSheet(const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::string serializeSheet(const Sheet&,std::stop_token={});
}
