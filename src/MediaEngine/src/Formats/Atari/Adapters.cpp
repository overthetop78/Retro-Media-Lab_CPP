#include "Adapters.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "../St/Geometry.h"
#include "../St/Transform.h"
#include "../St/Writer.h"
#include "../St/Constants.h"
#include "../Msa/Msa.h"
#include "../Msa/Constants.h"
#include "../Atr/Atr.h"
#include "../Atr/Constants.h"
#include "../Atx/Atx.h"
#include "../Atx/Constants.h"
#include "../FormatIds.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Atari {
namespace M=MediaFileSystems;
namespace {
std::vector<std::uint8_t> signatureBytes(std::uint32_t value,std::size_t count,bool bigEndian){std::vector<std::uint8_t> bytes(count);for(std::size_t index=0;index<count;++index){const auto shift=(bigEndian?count-1-index:index)*std::numeric_limits<std::uint8_t>::digits;bytes[index]=std::uint8_t((value>>shift)&std::numeric_limits<std::uint8_t>::max());}return bytes;}
std::vector<std::string> stIds(){std::vector<std::string> ids;for(const auto& profile:St::Constants::Profiles)ids.emplace_back(profile.id);return ids;}
bool supports(Kind kind,std::string_view format){
    if(kind==Kind::St||kind==Kind::Msa)return Primitives::ordinalIdentifierStartsWithIgnoreCase(format,Formats::AtariStPrefix);
    if(kind==Kind::Atr)return Primitives::ordinalIdentifierStartsWithIgnoreCase(format,Constants::AtrPrefix)||sameIdentifier(format,Formats::Atari90)||sameIdentifier(format,Formats::Atari130)||sameIdentifier(format,Formats::Atari140)||sameIdentifier(format,Formats::Atari180);
    return sameIdentifier(format,Formats::AtariAtx);
}
bool probe(Kind kind,const RecognitionContext& context,std::stop_token token){
    if(context.requestedFormatId()&&!supports(kind,*context.requestedFormatId()))return false;
    if(kind==Kind::Msa){if(context.length()<std::int64_t(Msa::Constants::HeaderSize))return false;const auto header=context.readHeaderAsync(sizeof(std::uint16_t),token).get();return (std::uint32_t(header[0])<<std::numeric_limits<std::uint8_t>::digits|header[1])==Msa::Constants::Signature;}
    if(kind==Kind::Atx){if(context.length()<std::int64_t(Atx::Constants::FileHeader))return false;const auto header=context.readHeaderAsync(sizeof(std::uint32_t),token).get();std::uint32_t value=0;for(std::size_t index=0;index<header.size();++index)value|=std::uint32_t(header[index])<<(index*std::numeric_limits<std::uint8_t>::digits);return value==Atx::Constants::Signature;}
    if(kind==Kind::St){if(context.length()==0||context.length()>std::numeric_limits<std::int32_t>::max()||context.length()%St::Constants::SectorSize)return false;try{St::detect(context.readBytesAsync(token).get());return true;}catch(const M::MediaError& error){if(error.kind()==M::ErrorKind::InvalidData)return false;throw;}}
    const auto bytes=context.readBytesAsync(token).get();try{Atr::validate(bytes);return true;}catch(const MediaError& error){switch(error.diagnostic().code){case DiagnosticCode::AtrHeader:case DiagnosticCode::AtrUnsupportedSize:case DiagnosticCode::AtrPayloadLength:case DiagnosticCode::AtrTruncatedPayload:return false;default:throw;}}
}
}
const ReaderCapabilities& Reader::capabilities()const{
    static const ReaderCapabilities st{stIds(),{std::string(Constants::StExtension)},{},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    static const ReaderCapabilities msa{stIds(),{std::string(Constants::MsaExtension)},{signatureBytes(Msa::Constants::Signature,sizeof(std::uint16_t),true)},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    static const ReaderCapabilities atr{{std::string(Formats::Atari90),std::string(Formats::Atari130),std::string(Formats::Atari140),std::string(Formats::Atari180)},{std::string(Constants::AtrExtension)},{signatureBytes(Atr::Constants::Signature,sizeof(std::uint16_t),false)},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    static const ReaderCapabilities atx{{std::string(Formats::AtariAtx)},{std::string(Constants::AtxExtension)},{signatureBytes(Atx::Constants::Signature,sizeof(std::uint32_t),false)},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    switch(kind_){case Kind::St:return st;case Kind::Msa:return msa;case Kind::Atr:return atr;case Kind::Atx:return atx;}return st;
}
bool Reader::supportsFormatId(std::string_view format)const{return supports(kind_,format);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{return probe(kind,context,token);});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,context,token]{const auto data=context.readBytesAsync(token).get();std::shared_ptr<const M::SectorImage> image;switch(kind){case Kind::St:image=St::read(data,token);break;case Kind::Msa:image=Msa::read(data,token);break;case Kind::Atr:image=Atr::read(data,token);break;case Kind::Atx:image=Atx::read(data,token);break;}return floppySectorDocument(context.source(),std::move(image));});}
std::shared_ptr<const ImageWriter> atrWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:Atari8Bit::Constants::Profiles)formats.emplace_back(profile.id);return sectorWriterAdapter(std::string(Constants::AtrWriterId),std::move(formats),{std::string(Constants::AtrExtension)},
    [](const auto& image,const auto&,auto format,auto){return Atr::canWrite(image,format);},
    [](const auto& image,const auto&,const std::filesystem::path& output,auto format,std::stop_token token){Atr::write(image,output,format,token);return std::vector<std::filesystem::path>{output};});}
std::shared_ptr<const ImageWriter> stWriterAdapter(){std::vector<std::string> formats;for(const auto& profile:St::Constants::Profiles)formats.emplace_back(profile.id);return sectorWriterAdapter(std::string(Constants::StWriterId),std::move(formats),{std::string(Constants::StExtension),std::string(Constants::MsaExtension)},
    [](const auto& image,const auto&,auto target,auto){return St::canTransformGeometry(image,target);},
    [](const auto& image,const MediaDocument& document,const std::filesystem::path& output,auto target,std::stop_token token){
        const auto& sourcePath=document.source().primaryPath();const auto sourceExtension=std::filesystem::path(std::u8string(sourcePath.begin(),sourcePath.end())).extension().u8string();const std::string sourceExt(sourceExtension.begin(),sourceExtension.end());
        const auto transformed=St::transform(image,target,Primitives::ordinalIdentifierEqualsIgnoreCase(sourceExt,Constants::ScpExtension),token);
        const auto extension=output.extension().u8string();const std::string ext(extension.begin(),extension.end());
        if(Primitives::ordinalIdentifierEqualsIgnoreCase(ext,Constants::MsaExtension))Msa::write(*transformed,output,token);else St::write(*transformed,output,token);return std::vector<std::filesystem::path>{output};});}
}
