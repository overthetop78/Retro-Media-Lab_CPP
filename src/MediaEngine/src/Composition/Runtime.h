#pragma once
#include "Recognition.h"
#include "Writing.h"
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
struct Services {
    MediaRecognitionComposition reading=MediaRecognitionComposition::createDefault();
    MediaWritingComposition writing=MediaWritingComposition::createDefault(reading.driveService());
};
std::shared_ptr<Services> defaultServices();
}
