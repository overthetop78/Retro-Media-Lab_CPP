#include "Internal.h"
#include "ConsoleConstants.h"
#include "../../Images/WriterRegistry.h"
namespace RetroMediaLab::MediaEngine::Cartridge {
const WriterCapabilities& ConsoleWriter::capabilities()const{
    static const auto caps=[]{WriterCapabilities result;result.id=ConsoleConstants::WriterId;result.representationKinds={RepresentationKind::Blocks};
        for(const auto& entry:ConsoleConstants::Extensions){result.producedFileExtensions.emplace_back(entry.extension);if(std::find(result.formatIds.begin(),result.formatIds.end(),entry.format)==result.formatIds.end())result.formatIds.emplace_back(entry.format);}return result;}();return caps;
}
bool ConsoleWriter::supportsTarget(std::string_view format,std::string_view extension)const{
    const auto normalized=normalizeExtension(extension);
    return std::any_of(ConsoleConstants::Extensions.begin(),ConsoleConstants::Extensions.end(),[&](const auto& entry){return sameIdentifier(format,entry.format)&&sameIdentifier(normalized,entry.extension);});
}
bool ConsoleWriter::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{
    const auto* blocks=dynamic_cast<const M::BlockRepresentation*>(&document.representation());std::int64_t length=0;
    return blocks&&blocks->capacity()>0&&supportsTarget(format,extension)&&M::MetadataNumbers::metadataNumber(document.metadata(),std::string(ConsoleConstants::StoredLengthKey),length)&&length>0&&length<=blocks->capacity();
}
std::future<std::vector<std::filesystem::path>> ConsoleWriter::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string format,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),format=std::move(format),token]{
        checkCancellation(token);const auto u8=output.extension().u8string();const std::string extension(u8.begin(),u8.end());
        if(!document||!ConsoleWriter{}.canWrite(*document,format,extension))throw MediaError(DiagnosticCode::ImageWriteRequest);
        const auto& blocks=dynamic_cast<const M::BlockRepresentation&>(document->representation());std::int64_t length=0;
        M::MetadataNumbers::metadataNumber(document->metadata(),std::string(ConsoleConstants::StoredLengthKey),length);
        writeAtomicImage(output,[&](const ImageOutput& write,std::stop_token stop){
            std::vector<std::uint8_t> bytes(static_cast<std::size_t>(std::min<std::int64_t>(length,ConsoleConstants::CopyBytes)));
            for(std::int64_t offset=0;offset<length;){checkCancellation(stop);const auto count=static_cast<std::size_t>(std::min<std::int64_t>(bytes.size(),length-offset));blocks.readExactlyAsync(offset,std::span(bytes).first(count),stop).get();write(std::span(bytes).first(count));offset+=count;}
        },token);return std::vector<std::filesystem::path>{output};
    });
}
}
