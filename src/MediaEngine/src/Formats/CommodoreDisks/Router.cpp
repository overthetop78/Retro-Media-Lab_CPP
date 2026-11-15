#include "FluxReader.h"
#include "../Commodore900/FluxReader.h"
#include "../Commodore900/Constants.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <utility>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const Scp::Image& image,std::optional<std::string_view> requested,TrackProgress progress,std::stop_token token){
    checkCancellation(token);
    if(requested==MediaFileSystems::Commodore::Constants::Format1581)return readMfmFlux(image,std::move(progress),token);
    if(requested==Commodore900::Constants::FormatId)return Commodore900::readFlux(image,std::move(progress),token);
    return readGcrFlux(image,requested,std::move(progress),token);
}
std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const std::filesystem::path& path,std::optional<std::string_view> requested,TrackProgress progress,std::stop_token token){
    checkCancellation(token);
    const auto source=fileSource(path);
    const auto image=Scp::read(*source,token);
    return readFlux(image,requested,std::move(progress),token);
}
}
