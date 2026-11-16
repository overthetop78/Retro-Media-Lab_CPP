#pragma once
#include "MediaConversion.h"
#include <optional>
namespace RetroMediaLab::MediaEngine {
struct SequentialConversionPlan {
 std::shared_ptr<const MediaDocument> targetDocument;
 std::vector<MessageInfo> diagnostics,losses;
};
RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<MediaConversionDestination>> availableSequentialConversionDestinationsAsync(std::shared_ptr<const MediaDocument>,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<SequentialConversionPlan> planSequentialConversionAsync(std::shared_ptr<const MediaDocument>,std::string target,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<MediaConversionResult> convertSequentialMediaAsync(std::filesystem::path source,std::filesystem::path output,std::string target,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},bool acceptLosses=false,std::stop_token={});
}
