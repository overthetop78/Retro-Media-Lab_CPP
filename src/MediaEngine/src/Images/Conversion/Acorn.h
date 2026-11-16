#pragma once
#include "Representation.h"
#include "../../Formats/Adf/AdfImages.h"
namespace RetroMediaLab::MediaEngine::AcornConversion {
enum class Mode {ContainerAdf,RawAdf,RawDfs};
class Converter final:public RepresentationConverter {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit Converter(Mode);
 RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> sourceKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> targetKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API bool canConvert(const MediaDocument&,std::string_view,RepresentationKind)const override;
 RETROMEDIALAB_MEDIAENGINE_API RepresentationConversionResult convert(std::shared_ptr<const MediaDocument>,std::string,RepresentationKind,const ConversionOptions& = {},const RepresentationProgress& = {},std::stop_token={})const override;
private:Mode mode_;
};
namespace Details {
std::shared_ptr<const MediaFileSystems::SectorImage> raw(const MediaDocument&,std::string_view,bool dfs,std::stop_token);
std::optional<Adf::Profile> container(const MediaDocument&,std::string_view,std::stop_token);
std::shared_ptr<const MediaFileSystems::SectorImage> containerImage(const MediaDocument&,const Adf::Profile&,std::stop_token);
}
}
