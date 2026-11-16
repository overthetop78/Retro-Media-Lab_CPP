#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
std::span<const FileSystemMigrationTarget> FileSystemMigrationTargetCatalog::all() noexcept {return CreationConstants::Targets;}
const FileSystemMigrationTarget& FileSystemMigrationTargetCatalog::get(std::string_view id) {
    for(const auto& target:all())if(Primitives::asciiIdentifierEqualsIgnoreCase(id,target.formatId))return target;
    throw MediaError(DiagnosticCode::MigrationTarget,{std::string(id)});
}
bool MigrationValidationReport::canExecute() const noexcept {return (metadataLossAccepted||losses.empty())&&std::none_of(losses.begin(),losses.end(),[](const auto& loss){return loss.isBlocking;});}
}
