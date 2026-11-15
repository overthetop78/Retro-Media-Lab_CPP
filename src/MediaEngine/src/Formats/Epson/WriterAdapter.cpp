#include "WriterAdapter.h"
#include "RawWriter.h"
#include "Geometry.h"
#include "Constants.h"
#include "../ImageDisk/ImageDisk.h"
#include "../ImageDisk/Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Epson {
std::shared_ptr<const ImageWriter> writerAdapter(){std::vector<std::string> formats;for(const auto& g:geometries())formats.push_back(g.formatId);return sectorWriterAdapter(std::string(Constants::WriterId),std::move(formats),{std::string(Constants::RawExtension),std::string(ImageDisk::Constants::Extension)},
    [](const auto& image,const auto&,auto target,auto){return Primitives::ordinalIdentifierEqualsIgnoreCase(image.geometry().formatId,target);},
    [](const auto& image,const MediaDocument& document,const std::filesystem::path& output,auto target,std::stop_token token){
        if(!Primitives::ordinalIdentifierEqualsIgnoreCase(image.geometry().formatId,target))throw MediaError(DiagnosticCode::EpsonWriteSourceFormat,{image.geometry().formatId,std::string(target)});
        const auto u8=output.extension().u8string();const std::string extension(u8.begin(),u8.end());
        if(Primitives::ordinalIdentifierEqualsIgnoreCase(extension,ImageDisk::Constants::Extension)){const auto& rep=static_cast<const SectorRepresentation&>(document.representation());const auto restored=ImageDisk::restore(rep.imageOwner(),document.metadata());if(restored)ImageDisk::write(*restored,output,token);else ImageDisk::write(ImageDisk::buildEpson(rep.imageOwner()),output,token);}
        else writeRaw(image,output,target,token);
        return std::vector<std::filesystem::path>{output};});}
}
