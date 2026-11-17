#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/MediaEngine/BuiltInDiskDefinitions.h>
#include <QStringList>
#include <span>
namespace RetroMediaLab::Infrastructure {
RETROMEDIALAB_INFRASTRUCTURE_API void validateGwOptions(std::span<const MediaEngine::EnabledOption>);
RETROMEDIALAB_INFRASTRUCTURE_API void validateGwTrackSpec(const QString&);
RETROMEDIALAB_INFRASTRUCTURE_API void validateGwPllSpec(const QString&);
RETROMEDIALAB_INFRASTRUCTURE_API QStringList tokenizeExpertArguments(const QString&);
RETROMEDIALAB_INFRASTRUCTURE_API QString captureSequence(qint64 value, bool alphabetic, int width);
RETROMEDIALAB_INFRASTRUCTURE_API bool parseCaptureSequence(const QString&, bool alphabetic, qint64&);
}
