#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <string>
#include <vector>
#include <string_view>
#include <functional>
namespace RetroMediaLab::MediaEngine::Decoding {
struct LocalizedText {std::string key,literal;std::vector<LocalizedText> arguments;};
using TextResolver=std::function<std::string(std::string_view)>;
RETROMEDIALAB_MEDIAENGINE_API LocalizedText literal(std::string);
RETROMEDIALAB_MEDIAENGINE_API LocalizedText message(std::string_view,std::vector<LocalizedText> = {});
RETROMEDIALAB_MEDIAENGINE_API std::string resolveText(const LocalizedText&,const TextResolver&);
}
