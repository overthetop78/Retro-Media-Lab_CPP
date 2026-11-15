#include "TwoImg.h"
#include "Constants.h"
#include "../Apple/Raw.h"
#include "../Nib/Nib.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::TwoImg {
namespace {
template<class T>T little(MediaFileSystems::Bytes data,std::size_t offset){T value=0;for(std::size_t i=0;i<sizeof(T);++i)value|=T(T(data[offset+i])<<(i*std::numeric_limits<std::uint8_t>::digits));return value;}
}
Header readHeader(MediaFileSystems::Bytes bytes){
    using namespace Constants;
    if(bytes.size()<MinimumHeaderSize)throw MediaError(DiagnosticCode::TwoImgHeader);
    if(!std::equal(Signature.begin(),Signature.end(),bytes.begin()+SignatureOffset))throw MediaError(DiagnosticCode::TwoImgSignature);
    const auto version=little<std::uint16_t>(bytes,VersionOffset);
    if(version!=SupportedVersion)throw MediaError(DiagnosticCode::TwoImgVersion,{std::to_string(version)});
    Header header{little<std::uint16_t>(bytes,HeaderSizeOffset),ImageFormat(little<std::uint32_t>(bytes,ImageFormatOffset)),little<std::uint32_t>(bytes,DataOffsetOffset),little<std::uint32_t>(bytes,DataLengthOffset)};
    if(header.dataOffset>MaximumSignedField||header.dataLength>MaximumSignedField)throw MediaError(DiagnosticCode::TwoImgOverflow);
    return header;
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){
    checkCancellation(token);const auto header=readHeader(bytes);
    if(header.headerLength<Constants::MinimumHeaderSize||header.dataOffset<header.headerLength||header.dataLength<Constants::MinimumDataLength
        ||header.dataLength>bytes.size()||header.dataOffset>bytes.size()-header.dataLength)throw MediaError(DiagnosticCode::TwoImgDataRange);
    const auto data=bytes.subspan(header.dataOffset,header.dataLength);
    switch(header.imageFormat){
        case ImageFormat::Dos:return Apple::readRaw(data,Constants::DosExtension,token).image;
        case ImageFormat::ProDos:return Apple::readRaw(data,Constants::ProDosExtension,token).image;
        case ImageFormat::Nib:return Nib::read(data,token);
    }
    throw MediaError(DiagnosticCode::TwoImgFormat,{std::to_string(std::uint32_t(header.imageFormat))});
}
}
