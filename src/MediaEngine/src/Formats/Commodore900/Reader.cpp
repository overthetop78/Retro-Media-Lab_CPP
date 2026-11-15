#include "CoherentImages.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Coherent.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Commodore900 {
using namespace Constants;
namespace {
std::vector<std::uint8_t> header(const std::shared_ptr<const MediaFileSystems::RandomAccessData>& source,std::stop_token token){
 checkCancellation(token);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
 std::vector<std::uint8_t> bytes(std::size_t(std::min(std::uint64_t(HeaderSize),source->length())));source->readExactly(0,bytes,token);return bytes;
}
}
bool canRead(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::optional<std::string_view> requested,std::stop_token token){
 checkCancellation(token);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
 if(requested&&!Internal::sameFormat(*requested))return false;
 const auto length=source->length();if(length>Capacity||length%SectorSize)return false;
 return MediaFileSystems::Coherent::looksLike(header(source,token));
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){
 const auto bytes=header(source,token);const auto length=source->length();if(!MediaFileSystems::Coherent::looksLike(bytes))throw MediaError(DiagnosticCode::C900Content,{std::to_string(length)});
 if(length%SectorSize)throw MediaError(DiagnosticCode::C900Alignment,{std::to_string(length),std::to_string(SectorSize)});
 const auto available=length/SectorSize,declared=std::uint64_t(MediaFileSystems::Coherent::declaredBlocks(bytes));
 if(declared<MinimumBlocks||declared>available)throw MediaError(DiagnosticCode::C900DeclaredBlocks,{std::to_string(declared),std::to_string(available)});
 if(available>BlockCount)throw MediaError(DiagnosticCode::C900Capacity,{std::to_string(available),std::to_string(BlockCount)});
 SourceSectorMetadata metadata{0,{},true};metadata.addressOf=[](auto logical){return addressOf(std::int64_t(logical));};
 return sourceSectorImage(std::move(source),geometry(),0,{false,length,available},std::move(metadata));
}
MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){const auto image=read(std::move(source),token);auto volume=MediaFileSystems::Coherent::read(*image,token);MediaFileSystems::analyzeEntries(image->geometry().formatId,volume,token);return volume;}
}
