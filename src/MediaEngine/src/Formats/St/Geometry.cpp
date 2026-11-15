#include "Geometry.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::St {
std::string formatId(std::int64_t capacity){if(capacity<0)throw MediaError(DiagnosticCode::StCapacityOverflow);return std::string(Formats::AtariStPrefix)+std::to_string(capacity/Constants::BytesPerKibibyte);}
MediaFileSystems::SectorGeometry geometry(std::int32_t cylinders,std::int32_t heads,std::int32_t sectors){using namespace Constants;if(cylinders<=0)throw MediaError(DiagnosticCode::StCylinderCount,{std::to_string(cylinders)});if(heads<=0)throw MediaError(DiagnosticCode::StHeadCount,{std::to_string(heads)});if(sectors<=0)throw MediaError(DiagnosticCode::StSectorCount,{std::to_string(sectors)});std::int64_t capacity=cylinders;for(auto factor:{heads,sectors,std::int32_t(SectorSize)}){if(capacity>MaxCapacity/factor)throw MediaError(DiagnosticCode::StCapacityOverflow);capacity*=factor;}return {formatId(capacity),SectorSize,std::uint32_t(cylinders),std::uint32_t(heads),std::uint32_t(sectors)};}
std::optional<MediaFileSystems::SectorGeometry> geometry(std::string_view id){for(const auto& p:Constants::Profiles)if(Primitives::asciiIdentifierEqualsIgnoreCase(id,p.id))return geometry(p.cylinders,p.heads,p.sectors);return {};}
}
