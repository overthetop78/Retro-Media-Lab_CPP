#include "Adapter.h"
#include "Constants.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Jfd {
namespace {
std::vector<std::uint8_t> loadFile(const std::string& path,std::stop_token token) {
    const auto source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));
    if(source->length()>Constants::MaximumLength)throw MediaError(DiagnosticCode::JfdLength);
    std::vector<std::uint8_t> bytes(std::size_t(source->length()));source->readExactly(0,bytes,token);return bytes;
}
bool invalidCompression(const MediaError& e) {
    switch(e.diagnostic().code){case DiagnosticCode::JfdLength:case DiagnosticCode::JfdExpansion:case DiagnosticCode::JfdCompression:return true;default:return false;}
}
}
Reader::Reader(FileLoader loader):loader_(loader?std::move(loader):loadFile){}
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token,loader=loader_]{
        using namespace Constants;
        if((context.requestedFormatId()&&!Primitives::ordinalIdentifierEqualsIgnoreCase(*context.requestedFormatId(),FormatId))||!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Extension)||context.length()<std::int64_t(Signature.size())||context.length()>std::int64_t(MaximumLength))return false;
        try{const auto bytes=loader(context.source()->primaryPath(),token);const auto unpacked=unpack(bytes,token);return unpacked.size()>=Signature.size()&&std::equal(Signature.begin(),Signature.end(),unpacked.begin());}
        catch(const MediaError& e){if(invalidCompression(e))return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token,loader=loader_]{
        if(context.length()>std::int64_t(Constants::MaximumLength))throw MediaError(DiagnosticCode::JfdLength);
        return readBytes(context.source(),loader(context.source()->primaryPath(),token),token);
    });
}
std::shared_ptr<const MediaDocument> readBytes(std::shared_ptr<const MediaSource> source,std::span<const std::uint8_t> bytes,std::stop_token token) {
    using namespace Constants;const auto image=decode(bytes,token);std::vector<MediaFileSystems::MessageRecipe> messages;
    if(image.hasRawTracks)messages.push_back({std::string(RawTracksMessage),{},std::string(TranslationDomain)});
    if(image.hasRotationalPositions)messages.push_back({std::string(PositionsMessage),{},std::string(TranslationDomain)});
    const bool gzip=bytes.size()>=GzipSignature.size()&&std::equal(GzipSignature.begin(),GzipSignature.end(),bytes.begin());
    return std::make_shared<const MediaDocument>(std::move(source),std::string(FormatId),MediaKind::Floppy,std::make_shared<const SectorRepresentation>(image.sectors),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),metadata(image,gzip));
}
}
