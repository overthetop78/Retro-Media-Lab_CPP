#include "Descriptor.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QStringDecoder>
#include <filesystem>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AlcoholMds {
namespace {
using namespace Constants;
std::vector<std::uint8_t> range(const RecognitionContext& context,std::int64_t offset,int length,std::stop_token token){checkCancellation(token);if(offset<0||length<0||context.length()<length||offset>context.length()-length)throw MediaError(DiagnosticCode::MdsDescriptor);return context.readAsync(offset,length,token).get();}
template<class T>T field(std::span<const std::uint8_t> bytes,std::size_t offset){return qFromLittleEndian<T>(bytes.data()+offset);}
std::string name(const RecognitionContext& context,std::int64_t offset,bool wide,std::stop_token token){if(offset<0||offset>=context.length())throw MediaError(DiagnosticCode::MdsDescriptor);const auto bytes=range(context,offset,int(std::min<std::int64_t>(MaximumNameBytes,context.length()-offset)),token);
    if(wide){std::size_t end=0;while(end+1<bytes.size()&&(bytes[end]||bytes[end+1]))end+=WideBytes;if(!end||end+1>=bytes.size())throw MediaError(DiagnosticCode::MdsDescriptor);QStringDecoder decoder(QStringConverter::Utf16LE,QStringConverter::Flag::ConvertInitialBom);const QString value=decoder(QByteArrayView(reinterpret_cast<const char*>(bytes.data()),qsizetype(end)));return value.toUtf8().toStdString();}
    const auto zero=std::find(bytes.begin(),bytes.end(),std::uint8_t{});if(zero==bytes.begin()||zero==bytes.end())throw MediaError(DiagnosticCode::MdsDescriptor);return QString::fromLatin1(reinterpret_cast<const char*>(bytes.data()),qsizetype(zero-bytes.begin())).toUtf8().toStdString();
}
std::filesystem::path pathOf(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
std::string dataPath(const RecognitionContext& context,std::int64_t footerOffset,std::stop_token token){const auto footer=range(context,footerOffset,TrackFooterSize,token);const auto value=name(context,field<std::uint32_t>(footer,FooterFileNameOffset),field<std::uint32_t>(footer,FooterWideCharacterOffset)!=0,token);auto descriptor=pathOf(context.source()->primaryPath());std::error_code error;descriptor=std::filesystem::absolute(descriptor,error).lexically_normal();if(error)throw MediaError(DiagnosticCode::MdsDescriptor);std::filesystem::path resolved;
    if(value.starts_with(WildcardPrefix)){resolved=descriptor;resolved.replace_extension(pathOf(std::string_view(value).substr(1)));}else resolved=descriptor.parent_path()/pathOf(value);
    resolved=std::filesystem::absolute(resolved,error).lexically_normal();if(error)throw MediaError(DiagnosticCode::MdsDescriptor);const auto text=resolved.u8string();return {text.begin(),text.end()};
}
SessionBlock session(const RecognitionContext& context,std::span<const std::uint8_t> bytes,std::stop_token token){const auto start=field<std::int32_t>(bytes,SessionStartOffset),end=field<std::int32_t>(bytes,SessionEndOffset);const auto number=field<std::uint16_t>(bytes,SessionNumberOffset),first=field<std::uint16_t>(bytes,SessionFirstTrackOffset),last=field<std::uint16_t>(bytes,SessionLastTrackOffset);const auto count=bytes[SessionTotalBlocksOffset];const auto blocks=field<std::uint32_t>(bytes,SessionTrackBlocksOffset);if(!number||!count||!first||last<first)throw MediaError(DiagnosticCode::MdsDescriptor);
    std::vector<TrackBlock> tracks;std::optional<std::int64_t> leadOut;
    for(int i=0;i<count;++i){const auto block=range(context,std::int64_t(blocks)+std::int64_t(i)*TrackBlockSize,TrackBlockSize,token);const auto trackNumber=block[TrackNumberOffset];if(trackNumber==LeadOut){const auto minutes=block[TrackMinuteOffset],seconds=block[TrackSecondOffset],frames=block[TrackFrameOffset];if(seconds>=SecondsPerMinute||frames>=FramesPerSecond)throw MediaError(DiagnosticCode::MdsDescriptor);leadOut=(std::int64_t(minutes)*SecondsPerMinute+seconds)*FramesPerSecond+frames;continue;}
        if(!field<std::uint32_t>(block,TrackExtraOffset))continue;const auto size=field<std::uint16_t>(block,TrackSectorSizeOffset);const auto footer=field<std::uint32_t>(block,TrackFooterOffset);if(!trackNumber||!size||!footer)throw MediaError(DiagnosticCode::MdsDescriptor);const auto offset=field<std::uint64_t>(block,TrackStartOffsetOffset);if(offset>MaximumSigned)throw MediaError(DiagnosticCode::MdsDescriptor);
        tracks.push_back({block[TrackSessionOffset],trackNumber,block[TrackModeOffset],block[TrackSubchannelModeOffset],block[TrackAdrControlOffset],size,field<std::uint32_t>(block,TrackStartSectorOffset),std::int64_t(offset),footer,dataPath(context,footer,token)});
    }
    std::int64_t resolved=leadOut.value_or(end);if(resolved<=start&&!tracks.empty())resolved=std::max_element(tracks.begin(),tracks.end(),[](const auto& a,const auto& b){return a.startSector<b.startSector;})->startSector+1;
    return SessionBlock(number,start,resolved,first,last,std::move(tracks));
}
}
SessionBlock::SessionBlock(int number,std::int64_t start,std::int64_t end,int first,int last,std::vector<TrackBlock> tracks):number_(number),start_(start),end_(end),first_(first),last_(last),tracks_(std::move(tracks)){if(number<=0||end<=start||first<=0||last<first||tracks_.empty())throw MediaError(DiagnosticCode::MdsDescriptor);}
Header readHeader(const RecognitionContext& context,std::stop_token token){using namespace Constants;const auto bytes=range(context,0,HeaderSize,token);if(!std::equal(Signature.begin(),Signature.end(),bytes.begin()+SignatureOffset))throw MediaError(DiagnosticCode::MdsHeader);const auto offset=field<std::uint32_t>(bytes,SessionsOffsetOffset);const auto count=field<std::uint16_t>(bytes,SessionCountOffset);if(!count||std::int64_t(offset)>context.length()-std::int64_t(count)*SessionBlockSize)throw MediaError(DiagnosticCode::MdsHeader);return {bytes[VersionOffset],bytes[VersionOffset+1],field<std::uint16_t>(bytes,MediumTypeOffset),count,offset};}
Descriptor readDescriptor(const RecognitionContext& context,std::stop_token token){using namespace Constants;Descriptor result;result.header=readHeader(context,token);if(result.header.major>MaximumMajorVersion||(result.header.mediumType&DvdMediumFlag))throw MediaError(DiagnosticCode::MdsProfile);for(int i=0;i<result.header.sessionCount;++i){const auto bytes=range(context,std::int64_t(result.header.sessionsOffset)+std::int64_t(i)*SessionBlockSize,SessionBlockSize,token);result.sessions.push_back(session(context,bytes,token));}return result;}
}
