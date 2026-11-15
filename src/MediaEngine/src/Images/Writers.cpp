#include "Writers.h"
#include "RecognitionContext.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
bool ImageWriter::supportsTarget(std::string_view format,std::string_view extension)const{
    const auto& value=capabilities();
    return std::any_of(value.formatIds.begin(),value.formatIds.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,format);})
        &&std::any_of(value.producedFileExtensions.begin(),value.producedFileExtensions.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,extension);});
}
}
