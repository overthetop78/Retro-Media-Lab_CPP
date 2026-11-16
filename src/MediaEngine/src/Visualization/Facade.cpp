#include "Providers.h"
#include "../Composition/Runtime.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
VisualizationDescriptor describeMediaVisualization(std::shared_ptr<const MediaDocument> document,std::stop_token token){if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);return RuntimeDetails::defaultServices()->visualization.describe(*document,token);}
std::shared_ptr<const MediaDocument> prepareMediaVisualization(std::shared_ptr<const MediaDocument> document,std::stop_token token){return RuntimeDetails::defaultServices()->visualization.prepare(std::move(document),token);}
std::future<bool> readBlockVisualizationElementAsync(std::shared_ptr<const MediaDocument> document,VisualizationElement element,std::stop_token token){return std::async(std::launch::async,[document=std::move(document),element=std::move(element),token]{if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);return Visualization::readBlockElement(*document,element,token);});}
}
