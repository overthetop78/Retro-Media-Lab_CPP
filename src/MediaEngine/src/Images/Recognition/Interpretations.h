#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <functional>
namespace RetroMediaLab::MediaEngine {
using SectorImageOwner=std::shared_ptr<const MediaFileSystems::SectorImage>;
using RecognizedImageNormalizer=std::function<SectorImageOwner(SectorImageOwner,std::string_view,std::stop_token)>;
using AdditionalImagePolicy=std::function<std::vector<SectorImageOwner>(SectorImageOwner,std::stop_token)>;
class SectorInterpretations final {
public:
 RETROMEDIALAB_MEDIAENGINE_API SectorInterpretations(std::vector<RecognizedImageNormalizer>,std::vector<AdditionalImagePolicy>);
 RETROMEDIALAB_MEDIAENGINE_API static SectorInterpretations createDefault(std::vector<std::string> supportedFormats);
 RETROMEDIALAB_MEDIAENGINE_API SectorImageOwner normalize(SectorImageOwner,std::string_view readerId,std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API std::vector<SectorImageOwner> additional(SectorImageOwner,std::stop_token={})const;
private:
 std::vector<RecognizedImageNormalizer> normalizers_;
 std::vector<AdditionalImagePolicy> additional_;
};
}
