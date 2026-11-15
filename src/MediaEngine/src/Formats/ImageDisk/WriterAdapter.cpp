#include "WriterAdapter.h"
#include "ImageDisk.h"
#include "Constants.h"
#include "../FormatIds.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/Representation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::ImageDisk {
std::shared_ptr<const ImageWriter> writerAdapter(){return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(Formats::Imd)},{std::string(Constants::Extension)},
    [](const auto&,const MediaDocument& document,auto,auto){return document.metadata().contains(std::string(Constants::MetadataKey));},
    [](const auto&,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){const auto& rep=static_cast<const SectorRepresentation&>(document.representation());const auto image=restore(rep.imageOwner(),document.metadata());if(!image)throw MediaError(DiagnosticCode::ImdMetadata);write(*image,output,token);return std::vector<std::filesystem::path>{output};});}
}
