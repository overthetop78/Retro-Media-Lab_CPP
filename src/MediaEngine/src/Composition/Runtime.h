#pragma once
#include "Recognition.h"
#include "Writing.h"
#include "Conversion.h"
#include "../Images/Conversion/Service.h"
#include "../Reconstruction/Scp/Composition.h"
#include "../Formats/Scp/Adapter.h"
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
struct Services {
    MediaRecognitionComposition reading=MediaRecognitionComposition::createDefault();
    MediaWritingComposition writing=MediaWritingComposition::createDefault(reading.driveService());
    ScpReconstruction::Composition sectors{reading.scpReader()};
    RepresentationConverterRegistry conversions=createRepresentationConverters(reading,sectors);
    MediaConversionService conversion{conversions,writing.registry(),writing.writingService()};
};
std::shared_ptr<Services> defaultServices();
}
