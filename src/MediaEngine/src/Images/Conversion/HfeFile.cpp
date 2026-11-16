#include "../../Composition/Runtime.h"
#include "../../Formats/Hfe/Hfe.h"
#include "../../Formats/Hfe/Constants.h"
#include "../../Formats/Scp/Constants.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/DriveOperation.h"
#include <RetroMediaLab/MediaEngine/FluxConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
bool canCreateSectorHfe(std::string_view format,std::string_view extension){return Primitives::ordinalIdentifierEqualsIgnoreCase(format,Hfe::Constants::FormatId)&&Primitives::ordinalIdentifierEqualsIgnoreCase(extension,Hfe::Constants::Extension);}
std::future<void> convertSectorFileToHfeAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),output=std::move(output),token]{
  checkCancellation(token);if(source.empty()||output.empty())throw MediaError(DiagnosticCode::ConversionRequest);
  const auto extension=source.extension().u8string();if(Primitives::ordinalIdentifierEqualsIgnoreCase(std::string(extension.begin(),extension.end()),Scp::Constants::Extension))throw MediaError(DiagnosticCode::ConversionIncompatibleFormat,{std::string(Hfe::Constants::FormatId),std::string(Scp::Constants::Extension)});
  const auto explored=services->disks.explore(source,{},{},token);DriveOperationScope activity(explored.document?explored.document->mediaSource()->options().drive.get():nullptr);
  const auto tracks=services->sectorEncoder.encode(*explored.image,token);Hfe::write(tracks,output,token);activity.finish();
 });
}
}
