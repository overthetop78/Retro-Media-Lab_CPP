#pragma once
#include "../Images/MediaDocument.h"
#include <RetroMediaLab/MediaEngine/MediaVisualization.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Visualization {
template<class T> const T& representation(const MediaDocument& document){const auto value=dynamic_cast<const T*>(&document.representation());if(!value)throw MediaError(DiagnosticCode::VisualizationProvider,{document.formatId(),std::to_string(int(document.representationInfo().representationKind()))});return *value;}
class Provider {
public:
 virtual ~Provider()=default;
 virtual std::span<const RepresentationKind> kinds()const noexcept=0;
 virtual bool canProvide(const MediaDocument&)const=0;
 virtual std::shared_ptr<const MediaDocument> prepare(std::shared_ptr<const MediaDocument> document,std::stop_token)const{return document;}
 virtual VisualizationDescriptor describe(const MediaDocument&,std::stop_token)const=0;
};
class Registry final {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit Registry(std::vector<std::shared_ptr<const Provider>>);
 RETROMEDIALAB_MEDIAENGINE_API static Registry createDefault();
 RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Provider> resolve(const MediaDocument&)const;
 RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> prepare(std::shared_ptr<const MediaDocument>,std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API VisualizationDescriptor describe(const MediaDocument&,std::stop_token={})const;
private:std::vector<std::shared_ptr<const Provider>> providers_;
};
std::shared_ptr<const Provider> fluxProvider();
std::shared_ptr<const Provider> sectorProvider();
std::shared_ptr<const Provider> blockProvider();
std::shared_ptr<const Provider> opticalProvider();
std::shared_ptr<const Provider> sequentialProvider();
RETROMEDIALAB_MEDIAENGINE_API bool readBlockElement(const MediaDocument&,const VisualizationElement&,std::stop_token={});
}
