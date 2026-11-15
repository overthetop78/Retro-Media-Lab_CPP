#pragma once
#include "SectorReader.h"
#include <exception>
namespace RetroMediaLab::MediaFileSystems {
struct FileSystemMatch {std::string readerId; FileSystemVolume volume;};
struct FileSystemReadFailure {std::string readerId; std::exception_ptr exception;};
struct FileSystemReadReport {
    std::vector<FileSystemMatch> matches;
    std::vector<FileSystemReadFailure> failures;
    bool hasMatches() const noexcept {return !matches.empty();}
    bool hasCorruption() const noexcept {return !failures.empty();}
    bool isUnrecognized() const noexcept {return matches.empty()&&failures.empty();}
};
struct SectorFileSystemCandidate {std::shared_ptr<const SectorImage> image; FileSystemMatch match;};
class SectorRegistry {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorRegistry();
    RETROMEDIALAB_MEDIAFILESYSTEMS_API explicit SectorRegistry(std::vector<std::shared_ptr<const SectorReader>>);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::shared_ptr<const SectorReader>> readers() const noexcept;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::span<const std::string> supportedFormatIds() const noexcept;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemReadReport readAll(const SectorImage&,std::stop_token={}) const;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemReadReport readCandidates(const SectorImage&,std::optional<std::string_view> = {},std::stop_token={}) const;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::optional<FileSystemMatch> tryRead(const SectorImage&,std::optional<std::string_view> = {},std::stop_token={}) const;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::vector<SectorFileSystemCandidate> readDistinctCandidates(std::span<const std::shared_ptr<const SectorImage>>,std::stop_token={}) const;
private:
    struct Impl;
    std::shared_ptr<const Impl> impl_;
};
}
