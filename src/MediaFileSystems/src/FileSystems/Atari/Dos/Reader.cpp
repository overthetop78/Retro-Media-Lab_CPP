#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AtariDos {
std::span<const std::string_view> catalogFormatIds() noexcept{return Constants::Formats;}
bool canRead(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto format=Identifiers::utf8(image.geometry().formatId);const bool supported=std::any_of(Constants::Formats.begin(),Constants::Formats.end(),[&](auto value){return format.compare(Identifiers::utf8(value),Qt::CaseInsensitive)==0;})||format.startsWith(Identifiers::utf8(Formats::AtariAtrPrefix),Qt::CaseInsensitive);
    if(!supported||image.blockCount()<Constants::LastDirectorySector)return false;if(image.blockCount()>std::uint64_t(std::numeric_limits<std::int32_t>::max()))return false;std::vector<std::uint8_t> vtoc;return trySector(image,Constants::VtocSector,vtoc,token)&&vtocLooksValid(vtoc,std::int32_t(image.blockCount()))&&locateDirectory(image,token).has_value();
}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(!canRead(image,token))throw MediaError(DiagnosticCode::AtariDosUnsupportedDirectory,{image.geometry().formatId,std::to_string(image.geometry().blockSize)});const auto directory=locateDirectory(image,token);if(!directory)throw MediaError(DiagnosticCode::AtariDosUnsupportedDirectory,{image.geometry().formatId,std::to_string(image.geometry().blockSize)});
    FileSystemVolume result;result.entries=readDirectory(image,*directory,result.warnings,token);const auto free=freeSectors(image,token);const bool myDos=Internal::extended(image,token);result.fileSystemId=myDos?Constants::MyDosId:Constants::Id;result.fileSystemDisplayName=result.fileSystemId;result.capacity=image.capacity();result.freeBytes=free?std::min(std::uint64_t(*free)*image.geometry().blockSize,image.capacity()):0;result.attributes=myDos?std::vector<std::string>{std::string(Constants::MyDosAttribute),std::string(Constants::ExtendedLinksAttribute)}:std::vector<std::string>{std::string(Constants::AtariDosAttribute)};if(!directory->isCanonical())result.attributes.push_back(std::string(Constants::RelocatedAttribute));return result;
}
}
