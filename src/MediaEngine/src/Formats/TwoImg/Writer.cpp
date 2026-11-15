#include "TwoImg.h"
#include "Constants.h"
#include "../Apple/Raw.h"
#include "../FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::TwoImg {
namespace {
template<class T>void put(std::span<std::uint8_t> bytes,std::size_t offset,T value){for(std::size_t i=0;i<sizeof(T);++i)bytes[offset+i]=std::uint8_t(value>>(i*std::numeric_limits<std::uint8_t>::digits));}
ImageFormat format(std::string_view target){
    using Primitives::asciiIdentifierEqualsIgnoreCase;
    if(asciiIdentifierEqualsIgnoreCase(target,Formats::AppleIIAppleDos113)||asciiIdentifierEqualsIgnoreCase(target,Formats::AppleIIAppleDos140))return ImageFormat::Dos;
    if(Apple::isProDos140(target)||asciiIdentifierEqualsIgnoreCase(target,Formats::AppleIIProDos800))return ImageFormat::ProDos;
    throw MediaError(DiagnosticCode::UnsupportedRawFormat,{std::string(target)+std::string(Constants::TargetSeparator)+std::string(Constants::ContainerExtension)});
}
}
std::vector<std::uint8_t> payload(const MediaFileSystems::SectorImage& image,std::string_view target){
    using namespace Constants;
    const auto imageFormat=format(target);const auto data=Apple::twoImgPayload(image,target);
    if(data.size()>MaximumSignedField-MinimumHeaderSize)throw MediaError(DiagnosticCode::TwoImgOverflow);
    std::vector<std::uint8_t> output(MinimumHeaderSize+data.size());
    std::copy(Signature.begin(),Signature.end(),output.begin()+SignatureOffset);std::copy(Creator.begin(),Creator.end(),output.begin()+CreatorOffset);
    put(output,HeaderSizeOffset,std::uint16_t(MinimumHeaderSize));put(output,VersionOffset,SupportedVersion);
    put(output,ImageFormatOffset,std::uint32_t(imageFormat));put(output,FlagsOffset,std::uint32_t(0));
    put(output,BlockCountOffset,imageFormat==ImageFormat::ProDos?std::uint32_t(data.size()/ProDosBlockBytes):std::uint32_t(0));
    put(output,DataOffsetOffset,std::uint32_t(MinimumHeaderSize));put(output,DataLengthOffset,std::uint32_t(data.size()));
    std::copy(data.begin(),data.end(),output.begin()+MinimumHeaderSize);return output;
}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view target,std::stop_token token){
    const auto bytes=payload(image,target);writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(bytes);},token);
}
}
