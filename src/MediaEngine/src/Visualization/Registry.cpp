#include "Providers.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Visualization {
Registry::Registry(std::vector<std::shared_ptr<const Provider>> providers):providers_(std::move(providers)){if(std::any_of(providers_.begin(),providers_.end(),[](const auto& provider){return !provider;}))throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Constants::Providers)});}
Registry Registry::createDefault(){return Registry({fluxProvider(),sectorProvider(),blockProvider(),opticalProvider(),sequentialProvider()});}
std::shared_ptr<const Provider> Registry::resolve(const MediaDocument& document)const{const auto kind=document.representationInfo().representationKind();for(const auto& provider:providers_){const auto kinds=provider->kinds();if(std::find(kinds.begin(),kinds.end(),kind)!=kinds.end()&&provider->canProvide(document))return provider;}return {};}
std::shared_ptr<const MediaDocument> Registry::prepare(std::shared_ptr<const MediaDocument> document,std::stop_token token)const{checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);const auto provider=resolve(*document);return provider?provider->prepare(std::move(document),token):document;}
VisualizationDescriptor Registry::describe(const MediaDocument& document,std::stop_token token)const{checkCancellation(token);const auto provider=resolve(document);if(!provider)throw MediaError(DiagnosticCode::VisualizationProvider,{document.formatId(),std::to_string(int(document.representationInfo().representationKind()))});return provider->describe(document,token);}
}
