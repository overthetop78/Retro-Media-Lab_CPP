#pragma once
#include "Service.h"
namespace RetroMediaLab::MediaEngine {
// Signal conversion does not require a recognized file protocol.
class TapeSignalRepresentationConverter final {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canConvert(const MediaDocument&,std::string_view target,RepresentationKind)const;
    RETROMEDIALAB_MEDIAENGINE_API SequentialMediaConversionPlan convert(std::shared_ptr<const MediaDocument>,std::string target,RepresentationKind,std::map<std::string,std::string> options={},std::stop_token={})const;
};
}
