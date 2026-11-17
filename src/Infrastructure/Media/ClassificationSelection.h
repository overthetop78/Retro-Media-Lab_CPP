#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/MediaEngine/DiskClassification.h>
#include <RetroMediaLab/MediaEngine/DiskExplorationTypes.h>
namespace RetroMediaLab::Infrastructure {
// Identifiers, never translated labels, are retained across theme reconstruction.
class ClassificationSelection final {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API ClassificationSelection();
    const MediaEngine::DiskClassificationCatalog& catalog()const noexcept{return catalog_;}
    const std::optional<std::string>& machine()const noexcept{return machine_;}
    const std::optional<std::string>& format()const noexcept{return format_;}
    const std::optional<std::string>& protection()const noexcept{return protection_;}
    const std::vector<std::string>& detectedFormats()const noexcept{return detected_;}
    bool automatic()const noexcept{return automatic_;}
    RETROMEDIALAB_INFRASTRUCTURE_API void setAutomatic(bool);
    RETROMEDIALAB_INFRASTRUCTURE_API void selectMachine(std::optional<std::string>);
    RETROMEDIALAB_INFRASTRUCTURE_API void selectFormat(std::optional<std::string>);
    RETROMEDIALAB_INFRASTRUCTURE_API void selectProtection(std::optional<std::string>);
    RETROMEDIALAB_INFRASTRUCTURE_API void detect(const MediaEngine::DiskImageInfo&);
    RETROMEDIALAB_INFRASTRUCTURE_API MediaEngine::DiskVisualizationClassification visualization()const;
    bool expanded=false;
private:
    void normalizeProtection();
    MediaEngine::DiskClassificationCatalog catalog_;
    std::optional<std::string> machine_,format_,protection_,detectedFormat_,detectedProtection_;
    std::vector<std::string> detected_;
    bool automatic_=true;
};
}
