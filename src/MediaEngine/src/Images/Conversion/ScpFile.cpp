#include <RetroMediaLab/MediaEngine/FluxConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Composition/Runtime.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Formats/Scp/SectorConverter.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/DriveOperation.h"
namespace RetroMediaLab::MediaEngine {
bool canCreateSectorScp(std::string_view format,std::string_view extension){return Primitives::ordinalIdentifierEqualsIgnoreCase(format,Scp::Constants::FormatId)&&Primitives::ordinalIdentifierEqualsIgnoreCase(extension,Scp::Constants::Extension);}
std::future<void> convertSectorFileToScpAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token token){
    return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),output=std::move(output),token]{
        checkCancellation(token);if(source.empty()||output.empty())throw MediaError(DiagnosticCode::ConversionRequest);
        SourceOptions options;const auto path=source.u8string();options.primaryPath.assign(path.begin(),path.end());
        auto document=services->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
        const auto* sectors=dynamic_cast<const SectorRepresentation*>(&document->representation());
        if(!sectors)throw MediaError(DiagnosticCode::RepresentationSectorsFlux,{document->formatId(),std::string(Scp::Constants::FormatId)});
        DriveOperationScope activity(document->mediaSource()->options().drive.get());
        Scp::SectorConverter(services->sectorEncoder).convert(sectors->image(),output,token);activity.finish();
    });
}
}
