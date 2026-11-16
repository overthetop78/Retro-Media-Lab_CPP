#pragma once
#include "FileSystemModels.h"
#include "InterpretationConstants.h"
#include <string_view>
namespace RetroMediaLab::MediaFileSystems {
class FileSystemAlternativePolicy final {
public:
    static constexpr auto MinimumWarningThreshold=InterpretationConstants::MinimumWarningThreshold;
    static constexpr auto MinimumValidatedCatalogEntries=InterpretationConstants::MinimumValidatedCatalogEntries;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static bool isCredible(const FileSystemVolume&);
};
struct FileSystemEvidence {
    std::size_t entryCount=0,warningCount=0;
    double decodeScore=0;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static FileSystemEvidence from(const FileSystemVolume&,double decodeScore);
    bool isBetterThan(const FileSystemEvidence& other)const noexcept {
        return entryCount>other.entryCount||(entryCount==other.entryCount&&warningCount<other.warningCount)
            ||(entryCount==other.entryCount&&warningCount==other.warningCount&&decodeScore>other.decodeScore);
    }
};
class FileSystemInterpretationIdentity final {
public:
    static constexpr char FieldSeparator=InterpretationConstants::FieldSeparator,EntrySeparator=InterpretationConstants::EntrySeparator,PathSeparator=InterpretationConstants::PathSeparator;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static std::string create(std::string_view format,const FileSystemVolume&);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static std::string createVolume(const FileSystemVolume&);
    RETROMEDIALAB_MEDIAFILESYSTEMS_API static std::string formatFamily(std::string_view);
};
}
