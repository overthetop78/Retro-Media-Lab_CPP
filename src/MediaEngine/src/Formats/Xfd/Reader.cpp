#include "Reader.h"
#include "Constants.h"
#include "../Atari8Bit/Geometry.h"
#include "../../Images/DocumentFactory.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Xfd {
namespace {
std::uint32_t length(const Atari8Bit::Constants::Profile& profile){return Atari8Bit::payloadLength(profile);}
const Atari8Bit::Constants::Profile* profileFor(std::int64_t size,const std::optional<std::string>& requested){for(const auto& p:Constants::Profiles)if((size==length(p)||size==std::int64_t(length(p))+Constants::PaddingLength)&&(!requested||Primitives::ordinalIdentifierEqualsIgnoreCase(*requested,p.id)))return &p;return nullptr;}
}
const ReaderCapabilities& Reader::capabilities()const{static const auto result=[]{ReaderCapabilities c;for(const auto& p:Constants::Profiles)c.formatIds.emplace_back(p.id);c.extensions={std::string(Constants::Extension)};c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return result;}
bool Reader::supportsFormatId(std::string_view id)const{return ImageReader::supportsFormatId(id);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{MediaFileSystems::checkCancellation(token);return profileFor(context.length(),context.requestedFormatId())!=nullptr;});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto bytes=context.readBytesAsync(token).get();const auto* profile=profileFor(std::int64_t(bytes.size()),context.requestedFormatId());if(!profile)throw MediaError(DiagnosticCode::XfdLength,{std::to_string(bytes.size())});
    const auto payload=length(*profile);if(bytes.size()!=payload&&!std::all_of(bytes.begin()+payload,bytes.end(),[](auto byte){return byte==0;}))throw MediaError(DiagnosticCode::XfdPadding);
    auto geometry=Atari8Bit::geometry(profile->sectorSize,profile->sectorCount);geometry.formatId=profile->id;
    std::vector<MediaFileSystems::SectorBlock> blocks;std::size_t offset=0;
    for(std::uint32_t logical=0;logical<profile->sectorCount;++logical){MediaFileSystems::checkCancellation(token);const auto count=profile->sectorSize>Constants::SingleSize&&logical<Constants::BootCount?Constants::SingleSize:profile->sectorSize;MediaFileSystems::SectorBlock b;b.logicalBlock=logical;b.address={logical/geometry.sectorsPerTrack,Constants::FirstHead,logical%geometry.sectorsPerTrack+Constants::FirstSector};b.data.assign(bytes.begin()+std::ptrdiff_t(offset),bytes.begin()+std::ptrdiff_t(offset+count));blocks.push_back(std::move(b));offset+=count;}
    MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=profile->sectorSize>Constants::SingleSize;options.capacity=payload;options.logicalBlockCount=profile->sectorCount;
    return floppySectorDocument(context.source(),sparseSectorImage(std::move(geometry),std::move(blocks),options));
});}
}
