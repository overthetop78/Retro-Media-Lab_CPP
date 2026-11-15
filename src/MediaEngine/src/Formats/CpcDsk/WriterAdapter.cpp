#include "WriterAdapter.h"
#include "CpcDsk.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::CpcDsk {
namespace {
bool standard(const Image& image){std::optional<std::size_t> firstLength;
    for(const auto& track:image.tracks){if(!track.present)return false;std::size_t length=Constants::TrackInformationBlockSize;
        for(const auto& sector:track.sectors){if(sector.data.size()!=(std::size_t(Constants::MinimumSectorSize)<<(sector.sizeCode&Constants::SectorSizeCodeMask)))return false;if(sector.data.size()>Constants::MaximumBuffer-length)throw MediaError(DiagnosticCode::CpcDskOverflow);length+=sector.data.size();}
        if(firstLength&&length!=*firstLength)return false;firstLength=length;
    }return true;
}
}
std::shared_ptr<const ImageWriter> writerAdapter(){return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(MediaFileSystems::Formats::AmstradCpc),std::string(MediaFileSystems::Formats::AmstradPcw),std::string(Constants::FormatId)},{std::string(Constants::Extensions.front()),std::string(Constants::Extensions.back())},
    [](const auto&,const auto&,auto,auto){return true;},
    [](const auto&,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){const auto& representation=static_cast<const SectorRepresentation&>(document.representation());const auto ext=output.extension().u8string();const std::string extension(ext.begin(),ext.end());const auto kind=Primitives::ordinalIdentifierEqualsIgnoreCase(extension,Constants::Extensions.back())?ContainerKind::Extended:ContainerKind::Standard;auto image=buildImage(representation.imageOwner(),kind,token);if(kind==ContainerKind::Standard&&!standard(image))image.kind=ContainerKind::Extended;write(image,output,token);return std::vector<std::filesystem::path>{output};});}
}
