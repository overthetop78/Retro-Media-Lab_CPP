#include "RawImages.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Exploration/SectorExploration.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Raw {
namespace {
std::vector<std::uint8_t> boot(const std::shared_ptr<const MediaFileSystems::RandomAccessData>& source,std::stop_token token){
    if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    checkCancellation(token);std::vector<std::uint8_t> bytes(std::size_t(std::min(std::uint64_t(Constants::SectorSize),source->length())));source->readExactly(0,bytes,token);checkCancellation(token);return bytes;
}
}
std::shared_ptr<const MediaFileSystems::SectorImage> readIbm(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){const auto bytes=boot(source,token);auto geometry=detectIbm(bytes,source->length());return sourceSectorImage(std::move(source),std::move(geometry),0,{},SourceSectorMetadata{1,{},true});}
std::shared_ptr<const MediaFileSystems::SectorImage> readMsx(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){const auto bytes=boot(source,token);const auto length=source->length();if(!looksLikeMsx(bytes,length))throw MediaError(DiagnosticCode::MsxInvalidBoot,{std::to_string(length)});auto geometry=msxFromCapacity(length,bytes[Constants::DescriptorOffset]);if(!geometry)throw MediaError(DiagnosticCode::MsxUnsupportedGeometry,{std::to_string(length),std::to_string(bytes[Constants::DescriptorOffset])});return sourceSectorImage(std::move(source),std::move(*geometry),0,{},SourceSectorMetadata{1,{},true});}
MediaFileSystems::FileSystemVolume exploreIbm(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){return exploreSectorImage(*readIbm(std::move(source),token),token);}
MediaFileSystems::FileSystemVolume exploreMsx(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){return exploreSectorImage(*readMsx(std::move(source),token),token);}
}
