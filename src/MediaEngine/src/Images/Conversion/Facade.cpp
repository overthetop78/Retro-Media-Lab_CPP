#include <RetroMediaLab/MediaEngine/MediaConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Composition/Runtime.h"
namespace RetroMediaLab::MediaEngine {
std::vector<MediaConversionDestination> availableConversionDestinations(std::shared_ptr<const MediaDocument> source){if(!source)throw MediaError(DiagnosticCode::ConversionRequest);return RuntimeDetails::defaultServices()->conversion.availableDestinations(*source);}
std::future<MediaConversionResult> convertMediaAsync(MediaConversionRequest request,std::stop_token token){return RuntimeDetails::defaultServices()->conversion.convertAsync(std::move(request),token);}
}
