#include "Container.h"
#include "Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <QByteArray>
#include <limits>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AcornWinchester {
namespace {
std::filesystem::path path(const std::string& value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
std::string utf8(const std::filesystem::path& value){const auto text=value.u8string();return {text.begin(),text.end()};}
bool descriptorExists(const std::filesystem::path& value){return QFileInfo(QString::fromStdU16String(value.u16string())).isFile();}
std::filesystem::path descriptorPath(const MediaSource& source){
    for(const auto& candidate:source.options().associatedPaths){const auto value=path(candidate);if(Primitives::ordinalIdentifierEqualsIgnoreCase(utf8(value.extension()),Constants::DescriptorExtension))return value;}
    auto result=path(source.primaryPath());return result.replace_extension(Constants::DescriptorExtension);
}
std::vector<std::uint8_t> readDescriptor(const std::filesystem::path& value,std::stop_token token){
    checkCancellation(token);if(!descriptorExists(value))return {};const auto source=fileSource(value);if(source->length()>std::uint64_t(std::numeric_limits<std::int32_t>::max()))throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::ContextFullRead);
    std::vector<std::uint8_t> bytes(std::size_t(source->length()));source->readExactly(0,bytes,token);return bytes;
}
bool basic(const RecognitionContext& context){return (!context.requestedFormatId()||Reader{}.supportsFormatId(*context.requestedFormatId()))&&context.length()>0&&context.length()%Constants::SectorSize==0;}
bool probe(const RecognitionContext& context,std::span<const std::uint8_t> descriptor,std::stop_token token){
    using namespace Constants;if(context.requestedFormatId())return true;if(!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Extension))return false;if(geometry(descriptor,context.length()))return true;if(context.length()<std::int64_t(SignatureMinimumLength))return false;
    const auto header=context.readAsync(HeaderSignatureOffset,std::int32_t(Signature.size()),token).get();if(!std::equal(header.begin(),header.end(),Signature.begin(),Signature.end()))return false;return header==context.readAsync(FooterSignatureOffset,std::int32_t(Signature.size()),token).get();
}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{std::string(DescriptorExtension)},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{checkCancellation(token);if(!basic(context))return false;if(context.requestedFormatId())return true;if(!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Constants::Extension))return false;return probe(context,readDescriptor(descriptorPath(*context.source()),token),token);});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;checkCancellation(token);if(!basic(context))throw MediaError(DiagnosticCode::AcornDatSource);
        const auto descriptorFile=descriptorPath(*context.source());const auto descriptor=readDescriptor(descriptorFile,token);if(!probe(context,descriptor,token))throw MediaError(DiagnosticCode::AcornDatSource);const auto shape=geometry(descriptor,context.length());
        if(shape&&context.length()>shape->capacity())throw MediaError(DiagnosticCode::AcornDatCapacity);const auto sector=shape?shape->sectorSize():SectorSize;if(context.length()%sector)throw MediaError(DiagnosticCode::AcornDatAlignment);const auto capacity=shape?shape->capacity():context.length();
        const auto source=fileSource(path(context.source()->primaryPath()));std::vector<DataRange> ranges{DataRange(0,context.length(),DataRangeKind::Stored,source)};std::vector<MediaFileSystems::MessageRecipe> messages;
        if(capacity>context.length()){ranges.emplace_back(context.length(),capacity-context.length(),DataRangeKind::Unavailable);messages.push_back({std::string(MissingSectorsMessage),{},std::string(TranslationDomain)});}
        if(!shape)messages.push_back({std::string(UnknownGeometryMessage),{},std::string(TranslationDomain)});
        auto options=context.source()->options();std::vector<std::string> associated;for(const auto& candidate:options.associatedPaths)if(std::none_of(associated.begin(),associated.end(),[&](const auto& value){return Primitives::ordinalIdentifierEqualsIgnoreCase(candidate,value);}))associated.push_back(candidate);
        if(descriptorExists(descriptorFile)){const auto candidate=utf8(descriptorFile);if(std::none_of(associated.begin(),associated.end(),[&](const auto& value){return Primitives::ordinalIdentifierEqualsIgnoreCase(candidate,value);}))associated.push_back(candidate);}options.associatedPaths=std::move(associated);
        auto representation=std::make_shared<BlockImage>(capacity,sector,std::move(ranges),shape);const auto base64=QByteArray(reinterpret_cast<const char*>(descriptor.data()),qsizetype(descriptor.size())).toBase64().toStdString();
        return std::make_shared<const MediaDocument>(std::make_shared<const MediaSource>(std::move(options)),std::string(FormatId),MediaKind::HardDisk,std::move(representation),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::map<std::string,std::string>{{std::string(LogicalBlockSizeKey),std::to_string(sector)},{std::string(DescriptorKey),base64}});
    });
}
}
