#pragma once
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include <RetroMediaLab/MediaFileSystems/MediaDocument.h>
namespace RetroMediaLab::MediaEngine {
struct SourceOptions {
    std::string primaryPath;
    std::vector<std::string> associatedPaths;
    std::optional<std::int64_t> knownLength;
    std::optional<std::string> requestedFormatId;
    std::optional<std::map<std::string,std::string>> parameters;
    std::shared_ptr<DriveMechanics> drive;
    bool trackDriveAccesses=true,driveWriting=false;
};
class MediaSource final:public MediaFileSystems::SourceDescriptor {
public:
    explicit MediaSource(SourceOptions options):options_(std::move(options)){}
    const std::string& primaryPath()const override{return options_.primaryPath;}
    std::optional<std::int64_t> knownLength()const override{return options_.knownLength;}
    const SourceOptions& options()const noexcept{return options_;}
    std::shared_ptr<const MediaSource> withDrive(std::shared_ptr<DriveMechanics> drive)const{auto options=options_;options.drive=std::move(drive);return std::make_shared<MediaSource>(std::move(options));}
private:SourceOptions options_;
};
}
