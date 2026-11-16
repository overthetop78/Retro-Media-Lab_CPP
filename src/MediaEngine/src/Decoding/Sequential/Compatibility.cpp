#include "Compatibility.h"
#include "../../Images/RecognitionContext.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
bool canDecodeTape(const MediaDocument& document,const std::optional<std::string>& machine,std::span<const std::string> formats,std::span<const std::string> machines) {
    return document.mediaKind()==MediaKind::Tape&&dynamic_cast<const SequentialImage*>(&document.representation())&&std::any_of(formats.begin(),formats.end(),[&](const auto& id){return sameIdentifier(document.formatId(),id);})&&(!machine||std::any_of(machines.begin(),machines.end(),[&](const auto& id){return sameIdentifier(*machine,id);}));
}
}
