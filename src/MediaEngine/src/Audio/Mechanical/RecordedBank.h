#pragma once
#include "RecordedRenderer.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
std::shared_ptr<RecordedRenderer> sharedRecordedBank();
RETROMEDIALAB_MEDIAENGINE_API std::unique_ptr<RecordedRenderer> createRecordedBank(std::filesystem::path root={},std::filesystem::path executable={});
}
