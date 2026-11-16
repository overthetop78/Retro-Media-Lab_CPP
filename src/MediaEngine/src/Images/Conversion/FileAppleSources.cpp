#include "Files.h"
#include "../DocumentFactory.h"
#include "../../Reconstruction/Apple/Reader.h"
#include "../../Formats/Apple/Adapter.h"
#include "../../Formats/Scp/Scp.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <array>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::SectorFileConversion {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
}
std::shared_ptr<const MediaDocument> Service::readApple(const std::filesystem::path& path,std::string_view target,Family family,std::stop_token token)const{
 checkCancellation(token);const bool nibble=family==Family::AppleNibble;
 bool scp=Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Scp::Constants::Extension);
 if(nibble){
  const auto bytes=fileSource(path);std::array<std::uint8_t,Scp::Constants::Signature.size()> signature{};
  scp=bytes->length()>=signature.size();if(scp){bytes->readExactly(0,signature,token);scp=Scp::hasSignature(signature);}
 }
 SourceOptions options;options.primaryPath=utf8(path);if(!nibble)options.requestedFormatId=std::string(target);const auto owner=std::make_shared<MediaSource>(std::move(options));
 if(scp)return floppySectorDocument(owner,scp_.appleReader().read(path,nibble?std::nullopt:std::optional<std::string>(target),{},token));
 return Apple::Reader().readAsync(RecognitionContext(owner),token).get();
}
}
