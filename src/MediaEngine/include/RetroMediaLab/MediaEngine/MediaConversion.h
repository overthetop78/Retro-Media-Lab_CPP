#pragma once
#include "Export.h"
#include "MessageInfo.h"
#include "MediaKind.h"
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
struct MediaConversionProgress {MessageInfo message;double percent;MediaKind mediaKind;};
using MediaConversionProgressCallback=std::function<void(const MediaConversionProgress&)>;
class MediaConversionRequest final {
public:
 RETROMEDIALAB_MEDIAENGINE_API MediaConversionRequest(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::string target,std::map<std::string,std::string> options={},MediaConversionProgressCallback={});
 std::shared_ptr<const MediaDocument> source;
 std::filesystem::path output;
 std::string targetFormatId;
 std::map<std::string,std::string> options;
 MediaConversionProgressCallback progress;
};
struct MediaConversionDestination {std::string formatId,extension,writerId;bool producesMultipleFiles=false;bool operator==(const MediaConversionDestination&)const=default;};
struct MediaConversionResult {std::vector<std::filesystem::path> producedFiles;std::vector<MessageInfo> diagnostics,losses;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<MediaConversionDestination> availableConversionDestinations(std::shared_ptr<const MediaDocument>);
RETROMEDIALAB_MEDIAENGINE_API std::future<MediaConversionResult> convertMediaAsync(MediaConversionRequest,std::stop_token={});
}
