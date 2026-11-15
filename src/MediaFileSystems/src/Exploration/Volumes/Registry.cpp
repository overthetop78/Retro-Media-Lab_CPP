#include <RetroMediaLab/MediaFileSystems/VolumeDetectors.h>
namespace RetroMediaLab::MediaFileSystems {
VolumeDetectorRegistry::VolumeDetectorRegistry(std::vector<std::shared_ptr<const VolumeDetector>> detectors):detectors_(std::move(detectors)) {
    for(const auto& detector:detectors_)if(!detector)throw MediaError(DiagnosticCode::VolumeDetectorNull);
}
std::future<VolumeDetectionResult> VolumeDetectorRegistry::detectAsync(std::shared_ptr<const ImageDocument> document,std::stop_token token)const {
    return std::async(std::launch::async,[document=std::move(document),detectors=detectors_,token] {
        if(!document)throw MediaError(DiagnosticCode::DocumentNull);
        const auto known=document->volumes();if(!known.empty())return VolumeDetectionResult{{known.begin(),known.end()},{}};
        for(const auto& detector:detectors){checkCancellation(token);if(auto result=detector->detectAsync(document,token).get())return std::move(*result);}
        return VolumeDetectionResult{{},{{DiagnosticCode::VolumeNoDetector,{}}}};
    });
}
}
