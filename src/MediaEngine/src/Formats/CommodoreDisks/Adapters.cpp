#include "Adapters.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Images/WriterAdapters.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <limits>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
namespace M=MediaFileSystems;
namespace {
ReaderCapabilities readerCaps(Kind kind,std::string_view extension){return {{std::string(formatId(kind))},{std::string(extension)},{},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};}
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
bool dosTarget(std::string_view format,std::string_view extension){return (same(format,formatId(Kind::D64))&&same(extension,Constants::D64Extension))||(same(format,formatId(Kind::D71))&&same(extension,Constants::D71Extension));}
}
const ReaderCapabilities& ImageReaderAdapter::capabilities()const{static const auto d64=readerCaps(Kind::D64,Constants::D64Extension),d71=readerCaps(Kind::D71,Constants::D71Extension),d81=readerCaps(Kind::D81,Constants::D81Extension);return kind_==Kind::D64?d64:kind_==Kind::D71?d71:d81;}
std::future<bool> ImageReaderAdapter::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{M::checkCancellation(token);if(context.requestedFormatId()&&!same(*context.requestedFormatId(),formatId(kind)))return false;return context.length()<=std::numeric_limits<std::int32_t>::max()&&findLayout(kind,std::uint64_t(context.length()));});}
std::future<std::shared_ptr<const MediaDocument>> ImageReaderAdapter::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{const auto data=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(kind,memorySource(data),token));});}
std::shared_ptr<const ImageWriter> dosWriterAdapter(){return sectorWriterAdapter(std::string(Constants::DosWriterId),{std::string(formatId(Kind::D64)),std::string(formatId(Kind::D71))},{std::string(Constants::D64Extension),std::string(Constants::D71Extension)},
    [](const auto& image,const auto&,auto target,auto extension){return dosTarget(target,extension)&&same(image.geometry().formatId,target);},
    [](const M::SectorImage& image,const auto&,const std::filesystem::path& output,auto target,std::stop_token token){
        if(!same(image.geometry().formatId,target))throw M::MediaError(M::DiagnosticCode::WriterCommodoreTransform,{image.geometry().formatId,std::string(target)});
        bool diagnostics=true;image.visitAvailableBlocks([&](auto logical){M::checkCancellation(token);const auto block=image.block(logical,token);diagnostics=block&&block->diagnosticCode.has_value();return diagnostics;},token);
        writeD64D71(image,output,diagnostics?ErrorMapMode::Preserve:ErrorMapMode::None,token);return std::vector<std::filesystem::path>{output};
    },false,dosTarget);}
std::shared_ptr<const ImageWriter> d81WriterAdapter(){return sectorWriterAdapter(std::string(Constants::D81WriterId),{std::string(formatId(Kind::D81))},{std::string(Constants::D81Extension)},
    [](const auto& image,const auto&,auto target,auto extension){return same(target,formatId(Kind::D81))&&same(extension,Constants::D81Extension)&&canWriteD81(image);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){writeD81(image,output,token);return std::vector<std::filesystem::path>{output};},false,
    [](auto target,auto extension){return same(target,formatId(Kind::D81))&&same(extension,Constants::D81Extension);});}
}
