#pragma once
#include "Representation.h"
#include "../../Reconstruction/Scp/Reader.h"
#include "../../Formats/Scp/SectorConverter.h"
#include "../../Formats/Definitions/DiskDefinitions.h"
namespace RetroMediaLab::MediaEngine {
class FluxToSectorConverter final:public RepresentationConverter {
public:
 RETROMEDIALAB_MEDIAENGINE_API FluxToSectorConverter(ScpReconstruction::Reader,Definitions::Identifiers);
 RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> sourceKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> targetKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API bool canConvert(const MediaDocument&,std::string_view,RepresentationKind)const override;
 RETROMEDIALAB_MEDIAENGINE_API RepresentationConversionResult convert(std::shared_ptr<const MediaDocument>,std::string,RepresentationKind,const ConversionOptions& = {},const RepresentationProgress& = {},std::stop_token={})const override;
private:ScpReconstruction::Reader reader_;Definitions::Identifiers formats_;
};
class SectorToFluxConverter final:public RepresentationConverter {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit SectorToFluxConverter(Scp::SectorConverter={});
 RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> sourceKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> targetKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API bool canConvert(const MediaDocument&,std::string_view,RepresentationKind)const override;
 RETROMEDIALAB_MEDIAENGINE_API RepresentationConversionResult convert(std::shared_ptr<const MediaDocument>,std::string,RepresentationKind,const ConversionOptions& = {},const RepresentationProgress& = {},std::stop_token={})const override;
private:Scp::SectorConverter converter_;
};
class HfeCellConverter final:public RepresentationConverter {
public:
 RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> sourceKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API std::span<const RepresentationKind> targetKinds()const noexcept override;
 RETROMEDIALAB_MEDIAENGINE_API bool canConvert(const MediaDocument&,std::string_view,RepresentationKind)const override;
 RETROMEDIALAB_MEDIAENGINE_API RepresentationConversionResult convert(std::shared_ptr<const MediaDocument>,std::string,RepresentationKind,const ConversionOptions& = {},const RepresentationProgress& = {},std::stop_token={})const override;
};
}
