#include "DiskCopy.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QByteArray>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskCopy {
namespace {
using namespace Constants;
std::string status(std::optional<bool> valid){return std::string(!valid?Missing:*valid?Valid:Invalid);}
const std::string* value(const Metadata& data,std::string_view key){const auto found=data.find(std::string(key));return found==data.end()?nullptr:&found->second;}
std::optional<bool> parseStatus(const Metadata& data,std::string_view key){
    const auto text=value(data,key);if(!text||*text==Missing)return {};
    if(*text==Valid)return true;if(*text==Invalid)return false;
    throw MediaError(DiagnosticCode::DiskCopyMetadataChecksum,{std::string(key)});
}
std::uint8_t parseByte(const std::string& text){
    if(text.empty())throw MediaError(DiagnosticCode::DiskCopyMetadataFormat);
    unsigned int result=0;
    for(const char c:text){
        if(c<FirstDigit||c>LastDigit)throw MediaError(DiagnosticCode::DiskCopyMetadataFormat);
        result=result*DecimalBase+unsigned(c-FirstDigit);if(result>MaximumByte)throw MediaError(DiagnosticCode::DiskCopyMetadataFormat);
    }
    return std::uint8_t(result);
}
}
Metadata metadata(const Image& image){
    using namespace Constants;
    return {{std::string(NameKey),QByteArray(reinterpret_cast<const char*>(image.name.data()),qsizetype(image.name.size())).toBase64().toStdString()},
        {std::string(DiskFormatKey),std::to_string(image.diskFormat)},{std::string(FormatByteKey),std::to_string(image.formatByte)},
        {std::string(DataChecksumKey),status(image.dataChecksumValid)},{std::string(TagChecksumKey),status(image.tagChecksumValid)}};
}
std::optional<Image> restore(std::shared_ptr<const SectorImage> image,const Metadata& data){
    using namespace Constants;
    if(!image)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    const auto name=value(data,NameKey),disk=value(data,DiskFormatKey),format=value(data,FormatByteKey);
    if(!name&&!disk&&!format)return {};
    if(!name||!disk||!format)throw MediaError(DiagnosticCode::DiskCopyMetadataIncomplete);
    QByteArray encoded;encoded.reserve(qsizetype(name->size()));
    for(char c:*name)if(Base64Whitespace.find(c)==std::string_view::npos)encoded.append(c);
    if(encoded.size()%Base64QuantumSize)throw MediaError(DiagnosticCode::DiskCopyMetadataName);
    const auto decoded=QByteArray::fromBase64Encoding(encoded,QByteArray::AbortOnBase64DecodingErrors);
    if(!decoded)throw MediaError(DiagnosticCode::DiskCopyMetadataName);
    if(decoded.decoded.size()>qsizetype(MaximumNameLength))throw MediaError(DiagnosticCode::DiskCopyMetadataFormat);
    const auto diskFormat=parseByte(*disk),formatByte=parseByte(*format);
    std::vector<std::uint8_t> bytes(decoded.decoded.begin(),decoded.decoded.end());
    return Image{std::move(image),std::move(bytes),diskFormat,formatByte,parseStatus(data,DataChecksumKey),parseStatus(data,TagChecksumKey)};
}
}
