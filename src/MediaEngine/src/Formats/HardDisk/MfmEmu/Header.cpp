#include "Header.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MfmEmu {
std::optional<Header> Header::decode(std::span<const std::uint8_t> source){
    using namespace Constants;if(source.size()<FixedHeaderLength||!std::equal(Signature.begin(),Signature.end(),source.begin()))return {};
    const auto read=[&](std::size_t offset){return qFromLittleEndian<std::uint32_t>(source.data()+offset);};const auto version=read(VersionOffset),major=version&MajorMask;
    if((version&TypeMask)!=FileType||major<FirstMajor||major>LatestMajor||(version&ReservedVersionMask))return {};
    const auto length=read(HeaderLengthOffset),data=read(TrackDataLengthOffset),trackHeader=read(TrackHeaderLengthOffset),cylinders=read(CylinderCountOffset),heads=read(HeadCountOffset),rate=read(SampleRateOffset);
    if(length<FixedHeaderLength||length>source.size()||length>MaximumSignedField||!data||data>MaximumSignedField||data%WordLength||trackHeader<MinimumTrackHeaderLength||trackHeader>MaximumSignedField||!cylinders||cylinders>MaximumSignedField||!heads||heads>MaximumSignedField||!rate)return {};
    const auto record=std::uint64_t(data)+trackHeader;if(std::uint64_t(cylinders)*heads>(MaximumLength-length-MinimumTrackHeaderLength)/record)return {};
    Header result;std::size_t cursor=FixedHeaderLength;
    const auto readString=[&](std::size_t& offset,std::size_t& size){if(cursor>length||length-cursor<WordLength)return false;const auto count=read(cursor);cursor+=WordLength;if(!count||count>length-cursor||source[cursor+count-StringTerminatorLength]!=0)return false;offset=cursor;size=count-StringTerminatorLength;cursor+=count;return true;};
    if((version&(TypeMask|MajorMask))>=StringVersion&&(!readString(result.commandOffset_,result.commandLength_)||!readString(result.noteOffset_,result.noteLength_)))return {};
    if(version>=StartTimeVersion){if(cursor>length||length-cursor<WordLength)return {};result.start_=read(cursor);}
    result.version_=version;result.length_=std::int32_t(length);result.dataLength_=std::int32_t(data);result.trackHeader_=std::int32_t(trackHeader);result.cylinders_=std::int32_t(cylinders);result.heads_=std::int32_t(heads);result.rate_=rate;result.bytes_.assign(source.begin(),source.begin()+length);return result;
}
Header Header::create(std::int32_t cylinders,std::int32_t heads,std::int32_t trackBytes,std::uint32_t rate,std::uint32_t start,std::string_view command,std::string_view note){
    using namespace Constants;if(command.find(Null)!=std::string_view::npos||note.find(Null)!=std::string_view::npos||command.size()>MaximumSignedField||note.size()>MaximumSignedField)throw MediaError(DiagnosticCode::MfmHeader);
    const auto commandLength=command.size()+StringTerminatorLength,noteLength=note.size()+StringTerminatorLength,length=FixedHeaderLength+WordLength*CreationExtraWords+commandLength+noteLength;if(length>MaximumSignedField)throw MediaError(DiagnosticCode::MfmHeader);std::vector<std::uint8_t> bytes(length);std::copy(Signature.begin(),Signature.end(),bytes.begin());
    const auto write=[&](std::size_t offset,std::uint32_t value){qToLittleEndian(value,bytes.data()+offset);};write(VersionOffset,CurrentVersion);write(HeaderLengthOffset,std::uint32_t(length));write(TrackDataLengthOffset,std::uint32_t(trackBytes));write(TrackHeaderLengthOffset,MinimumTrackHeaderLength);write(CylinderCountOffset,std::uint32_t(cylinders));write(HeadCountOffset,std::uint32_t(heads));write(SampleRateOffset,rate);
    auto cursor=FixedHeaderLength;write(cursor,std::uint32_t(commandLength));cursor+=WordLength;std::copy(command.begin(),command.end(),bytes.begin()+cursor);cursor+=commandLength;write(cursor,std::uint32_t(noteLength));cursor+=WordLength;std::copy(note.begin(),note.end(),bytes.begin()+cursor);cursor+=noteLength;write(cursor,start);
    auto result=decode(bytes);if(!result)throw MediaError(DiagnosticCode::MfmHeader);return std::move(*result);
}
std::uint64_t Header::trackOffset(std::int32_t cylinder,std::int32_t head)const{if(cylinder<0||cylinder>=cylinders_||head<0||head>=heads_)throw MediaError(DiagnosticCode::MfmTrackRange);return length_+(std::uint64_t(cylinder)*heads_+head)*trackRecordLength();}
bool Header::trackAddress(std::span<const std::uint8_t> bytes,std::int32_t& cylinder,std::int32_t& head,bool& end)const{
    using namespace Constants;cylinder=head=0;end=false;if(bytes.size()<MinimumTrackHeaderLength||qFromLittleEndian<std::uint32_t>(bytes.data())!=TrackMarker)return false;const auto c=qFromLittleEndian<std::int32_t>(bytes.data()+TrackCylinderOffset),h=qFromLittleEndian<std::int32_t>(bytes.data()+TrackHeadOffset);if(c==EndAddress&&h==EndAddress){cylinder=head=EndAddress;end=true;return true;}if(bytes.size()<std::size_t(trackHeader_)||c<0||c>=cylinders_||h<0||h>=heads_)return false;cylinder=c;head=h;return true;
}
}
