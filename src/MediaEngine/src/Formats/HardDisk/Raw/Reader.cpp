#include "Reader.h"
#include <RetroMediaLab/MediaFileSystems/AtariHardDisk.h>
#include "Format.h"
#include "../../../Models/Blocks/BlockImage.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Rdb.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HardDiskRaw {
namespace {
using namespace Constants;
bool gpt(const RecognitionContext& context,std::int32_t blockSize,std::stop_token token){
    if(!compatible(context.length(),blockSize)||context.length()<blockSize+std::int64_t(GptSignature.size()))return false;
    const auto header=context.readAsync(blockSize,std::int32_t(GptSignature.size()),token).get();return std::equal(header.begin(),header.end(),GptSignature.begin(),GptSignature.end());
}
std::optional<std::int32_t> structuredBlockSize(const RecognitionContext& context,std::stop_token token){
    if(gpt(context,LargeBlockSize,token))return LargeBlockSize;
    if(gpt(context,LegacyBlockSize,token))return LegacyBlockSize;
    if(context.length()>=LegacyBlockSize&&compatible(context.length(),LegacyBlockSize)){
        namespace Rdb = MediaFileSystems::Rdb;
        const auto probeBytes = std::min<std::int64_t>(context.length(), Rdb::HeaderSearchSectors * Rdb::SectorSize);
        const auto probe = context.readAsync(0, std::int32_t(probeBytes), token).get();
        const MediaFileSystems::Bytes headers(probe);
        for (std::size_t offset = 0; offset + Rdb::SectorSize <= headers.size(); offset += Rdb::SectorSize)
            if (Rdb::validHeader(headers.subspan(offset, Rdb::SectorSize))) return LegacyBlockSize;
        if (MediaFileSystems::AtariHardDisk::probe(headers, context.length())) return LegacyBlockSize;
        const auto signature = headers.subspan(MbrSignatureOffset, MbrSignature.size());
        if(std::equal(signature.begin(),signature.end(),MbrSignature.begin(),MbrSignature.end()))return LegacyBlockSize;
    }
    return {};
}
std::int32_t resolve(const RecognitionContext& context,std::stop_token token){
    if(const auto size=structuredBlockSize(context,token))return *size;
    const auto requested=context.requestedFormatId();if(requested&&Primitives::ordinalIdentifierEqualsIgnoreCase(*requested,FormatId))return LegacyBlockSize;
    throw MediaError(DiagnosticCode::HardDiskRawIntent);
}
}
const ReaderCapabilities& Reader::capabilities()const{static const auto value=[]{ReaderCapabilities c;c.formatIds={std::string(Constants::FormatId)};for(auto extension:Constants::Extensions)c.extensions.emplace_back(extension);c.mediaKinds={MediaKind::HardDisk};c.representationKinds={RepresentationKind::Blocks};return c;}();return value;}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto requested=context.requestedFormatId();if(requested){if(!Reader{}.supportsFormatId(*requested))return false;return compatible(context.length(),resolve(context,token));}
    return structuredBlockSize(context,token).has_value();
});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    const auto blockSize=resolve(context,token);if(!compatible(context.length(),blockSize))throw MediaError(DiagnosticCode::HardDiskRawAlignment,{std::to_string(blockSize)});
    std::u8string path;for(auto byte:context.source()->primaryPath())path.push_back(char8_t(byte));const auto source=fileSource(std::filesystem::path(path));
    if(source->length()!=std::uint64_t(context.length()))throw MediaError(DiagnosticCode::HardDiskRawChanged);
    auto blocks=std::make_shared<BlockImage>(context.length(),blockSize,std::vector<DataRange>{DataRange(0,context.length(),DataRangeKind::Stored,source)});
    return std::make_shared<MediaDocument>(context.source(),std::string(Constants::FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(Constants::LogicalBlockSizeKey),std::to_string(blockSize)}});
});}
}
