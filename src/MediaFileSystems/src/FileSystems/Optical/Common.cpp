#include "Common.h"
#include "Constants.h"
#include <QStringConverter>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Optical {
Bytes slice(Bytes bytes,std::size_t offset,std::size_t length){if(offset>bytes.size()||length>bytes.size()-offset)throw MediaError(DiagnosticCode::OpticalBounds);return bytes.subspan(offset,length);}
std::shared_ptr<const OpticalTrack> resolve(const ImageDocument& document,const VolumeDescriptor& volume,DiagnosticCode required,DiagnosticCode absent){
    const auto representation=dynamic_cast<const OpticalRepresentation*>(&document.representation());
    if(!representation)throw MediaError(required);
    const auto tracks=representation->tracks();if(!tracks)throw MediaError(required);
    for(const auto& track:*tracks)if(track&&!track->isAudio()&&(!volume.sessionNumber()||track->sessionNumber()==*volume.sessionNumber())&&(!volume.trackNumber()||track->trackNumber()==*volume.trackNumber()))return track;
    throw MediaError(absent);
}
std::vector<std::uint8_t> sector(const OpticalTrack& track,std::int64_t relative,std::stop_token token){checkCancellation(token);return track.readUserDataAsync(relative,token).get();}
std::string ascii(Bytes bytes){std::string value;value.reserve(bytes.size());for(const auto byte:bytes)value.push_back(byte>AsciiMaximum?AsciiReplacement:char(byte));return value;}
QString utf16be(Bytes bytes){QStringDecoder decoder(QStringConverter::Utf16BE,QStringConverter::Flag::Stateless|QStringConverter::Flag::ConvertInitialBom);return decoder(QByteArrayView(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size())));}
std::string utf8OrLatin1(Bytes bytes){const auto view=QByteArrayView(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));QStringDecoder decoder(QStringConverter::Utf8,QStringConverter::Flag::Stateless|QStringConverter::Flag::ConvertInitialBom);const QString result=decoder(view);return (decoder.hasError()?QString::fromLatin1(view):result).toUtf8().toStdString();}
std::uint32_t add(std::uint32_t left,std::uint32_t right){if(right>std::numeric_limits<std::uint32_t>::max()-left)throw MediaError(DiagnosticCode::OpticalOverflow);return left+right;}
int integer(std::uint64_t value){if(value>std::uint64_t(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::OpticalOverflow);return int(value);}
bool probeFailure(const MediaError& error,bool overflow){return error.kind()==ErrorKind::InvalidData||error.kind()==ErrorKind::Unsupported||error.kind()==ErrorKind::ArgumentOutOfRange||(overflow&&error.kind()==ErrorKind::Overflow);}
}
