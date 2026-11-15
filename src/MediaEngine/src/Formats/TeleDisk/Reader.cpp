#include "Reader.h"
#include "Internal.h"
#include "Advanced.h"
#include "SectorCodec.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::TeleDisk {
namespace {
using namespace Constants;
std::string number(std::size_t n){return std::to_string(n);}
std::string hex(std::uint16_t n,int width){return QString::number(n,HexBase).toUpper().rightJustified(width,QChar(HexPadding)).toStdString();}
void require(MediaFileSystems::Bytes data,std::size_t offset,std::size_t count,DiagnosticCode code){if(offset>data.size()||count>data.size()-offset)throw MediaError(code,{number(offset),number(count),number(offset>data.size()?0:data.size()-offset)});}
std::uint16_t word(MediaFileSystems::Bytes data,std::size_t offset){return std::uint16_t(data[offset]|std::uint16_t(data[offset+1])<<BitsPerByte);}
bool starts(MediaFileSystems::Bytes data,std::string_view signature){return data.size()>=signature.size()&&std::equal(signature.begin(),signature.end(),data.begin());}
Sector readSector(MediaFileSystems::Bytes data,std::size_t& offset,std::stop_token token){
    require(data,offset,SectorHeaderSize,DiagnosticCode::Td0SectorTruncated);const auto h=data.subspan(offset,SectorHeaderSize);offset+=SectorHeaderSize;
    Sector sector{h[SectorCylinderOffset],h[SectorHeadOffset],h[SectorNumberOffset],h[SectorSizeCodeOffset],h[SectorFlagsOffset],{}};
    auto address=std::vector<std::string>{number(sector.cylinder),number(sector.head),number(sector.number)};
    if(sector.sizeCode>MaximumSectorSizeCode){address.push_back(number(sector.sizeCode));throw MediaError(DiagnosticCode::Td0SizeCode,address);}
    const auto length=BaseSectorSize<<sector.sizeCode;
    if(!(sector.flags&UnavailableMask)){
        require(data,offset,SectorDataHeaderSize,DiagnosticCode::Td0DataHeaderTruncated);const auto encodedLength=word(data,offset+EncodedLengthOffset);const auto encoding=SectorEncoding(data[offset+EncodingOffset]);offset+=SectorDataHeaderSize;
        if(!encodedLength){address.push_back(number(offset));throw MediaError(DiagnosticCode::Td0MissingData,address);}
        const auto payloadLength=encodedLength-EncodingFieldSize;require(data,offset,payloadLength,DiagnosticCode::Td0DataTruncated);
        sector.data=decodeSector(data.subspan(offset,payloadLength),encoding,length,sector.cylinder,sector.head,sector.number);offset+=payloadLength;
    }
    const auto stored=h[SectorCrcOffset];const auto calculated=sector.data?std::uint8_t(crc(*sector.data,CrcInitial,token)):std::uint8_t(CrcInitial);
    if(stored!=calculated){address.push_back(hex(stored,ByteHexWidth));address.push_back(hex(calculated,ByteHexWidth));throw MediaError(DiagnosticCode::Td0SectorCrc,address);}
    return sector;
}
}
Image readDetailed(MediaFileSystems::Bytes input,std::stop_token token){
    using namespace Constants;MediaFileSystems::checkCancellation(token);require(input,0,HeaderSize,DiagnosticCode::Td0ImageTruncated);
    std::vector<std::uint8_t> normal;auto data=input;
    if(starts(data,AdvancedSignature)){
        const auto expanded=decompressAdvanced(data.subspan(HeaderSize),token);normal.assign(data.begin(),data.begin()+HeaderSize);normal.insert(normal.end(),expanded.begin(),expanded.end());std::copy(Signature.begin(),Signature.end(),normal.begin());const auto headerCrc=crc(MediaFileSystems::Bytes(normal).first(HeaderCrcOffset),CrcInitial,token);normal[HeaderCrcOffset]=std::uint8_t(headerCrc);normal[HeaderCrcOffset+1]=std::uint8_t(headerCrc>>BitsPerByte);data=normal;
    }
    if(!starts(data,Signature))throw MediaError(DiagnosticCode::Td0Signature);
    const auto stored=word(data,HeaderCrcOffset),calculated=crc(data.first(HeaderCrcOffset),CrcInitial,token);
    if(stored!=calculated)throw MediaError(DiagnosticCode::Td0HeaderCrc,{hex(stored,WordHexWidth),hex(calculated,WordHexWidth)});
    Image image;image.header={data[SequenceOffset],data[CheckSignatureOffset],data[VersionOffset],data[DataRateOffset],data[DriveTypeOffset],data[SteppingOffset],data[DosModeOffset],data[SurfaceCountOffset]};std::size_t offset=HeaderSize;
    if(image.header.trackDensity&CommentPresentMask){
        require(data,offset,CommentHeaderSize,DiagnosticCode::Td0CommentHeaderTruncated);const auto h=data.subspan(offset,CommentHeaderSize);const auto storedComment=word(h,CommentCrcOffset),length=word(h,CommentLengthOffset);offset+=CommentHeaderSize;
        require(data,offset,length,DiagnosticCode::Td0CommentTruncated);const auto bytes=data.subspan(offset,length);const auto actual=crc(bytes,crc(h.subspan(CommentLengthOffset),CrcInitial,token),token);
        if(storedComment!=actual)throw MediaError(DiagnosticCode::Td0CommentCrc,{hex(storedComment,WordHexWidth),hex(actual,WordHexWidth)});
        image.comment=Comment{h[CommentYearOffset],h[CommentMonthOffset],h[CommentDayOffset],h[CommentHourOffset],h[CommentMinuteOffset],h[CommentSecondOffset],{bytes.begin(),bytes.end()}};offset+=length;
    }
    std::size_t sectors{};
    for(;;){
        MediaFileSystems::checkCancellation(token);require(data,offset,ByteFieldSize,DiagnosticCode::Td0TrackTruncated);const auto count=data[offset+TrackSectorCountOffset];if(count==EndOfTracks)break;
        require(data,offset,TrackHeaderSize,DiagnosticCode::Td0TrackTruncated);const auto h=data.subspan(offset,TrackHeaderSize);const auto storedTrack=h[TrackCrcOffset],actual=std::uint8_t(crc(h.first(TrackCrcOffset),CrcInitial,token));
        if(storedTrack!=actual)throw MediaError(DiagnosticCode::Td0TrackCrc,{number(h[TrackCylinderOffset]),number(h[TrackHeadOffset]&HeadMask),hex(storedTrack,ByteHexWidth),hex(actual,ByteHexWidth)});
        Track track{h[TrackCylinderOffset],h[TrackHeadOffset],{}};offset+=TrackHeaderSize;track.sectors.reserve(count);
        for(std::size_t index=0;index<count;++index){MediaFileSystems::checkCancellation(token);track.sectors.push_back(readSector(data,offset,token));}
        sectors+=count;image.tracks.push_back(std::move(track));
    }
    if(!sectors)throw MediaError(DiagnosticCode::Td0NoSectors);
    image.sectorImage=buildSectorImage(image.tracks,image.header.dataRate,token);return image;
}
}
