#include "Container.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <array>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MfmEmu {
std::shared_ptr<const Container> Container::open(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(!source||source->length()<FixedHeaderLength)throw MediaError(DiagnosticCode::MfmHeader);std::array<std::uint8_t,FixedHeaderLength> prefix{};source->readExactly(0,prefix,token);if(!std::equal(Signature.begin(),Signature.end(),prefix.begin()))throw MediaError(DiagnosticCode::MfmHeader);const auto length=qFromLittleEndian<std::uint32_t>(prefix.data()+HeaderLengthOffset);
    if(length<FixedHeaderLength||length>MaximumSignedField||length>source->length())throw MediaError(DiagnosticCode::MfmHeader);std::vector<std::uint8_t> bytes(length);std::copy(prefix.begin(),prefix.end(),bytes.begin());source->readExactly(FixedHeaderLength,std::span(bytes).subspan(FixedHeaderLength),token);auto header=Header::decode(bytes);if(!header)throw MediaError(DiagnosticCode::MfmHeader);
    if(source->length()<MinimumTrackHeaderLength||header->endMarkerOffset()!=source->length()-MinimumTrackHeaderLength)throw MediaError(DiagnosticCode::MfmLength);std::array<std::uint8_t,MinimumTrackHeaderLength> end{};source->readExactly(header->endMarkerOffset(),end,token);std::int32_t c{},h{};bool eof=false;if(!header->trackAddress(end,c,h,eof)||!eof)throw MediaError(DiagnosticCode::MfmEnd);
    return std::shared_ptr<const Container>(new Container(std::move(source),std::make_shared<const Header>(std::move(*header))));
}
std::shared_ptr<const Container> Container::blank(Header header){auto owner=std::make_shared<const Header>(std::move(header));return std::shared_ptr<const Container>(new Container(blankData(owner),owner));}
std::shared_ptr<const Container> Container::formatted(Header header,HardDiskGeometry geometry,std::vector<std::uint8_t> prefix){auto owner=std::make_shared<const Header>(std::move(header));return std::shared_ptr<const Container>(new Container(formattedData(owner,std::move(geometry),std::move(prefix)),owner));}
std::shared_ptr<const Container> Container::withTrackBytes(std::int32_t cylinder,std::int32_t head,std::span<const std::uint8_t> bytes)const{
    const auto offset=header().trackOffset(cylinder,head);if(bytes.size()!=std::size_t(header().trackDataLength()))throw MediaError(DiagnosticCode::MfmTrackRange);auto changes=changes_;changes[offset+header().trackHeaderLength()]=std::make_shared<const std::vector<std::uint8_t>>(bytes.begin(),bytes.end());return std::shared_ptr<const Container>(new Container(source_,header_,std::move(changes)));
}
void Container::validateTrack(std::uint64_t offset,std::int32_t cylinder,std::int32_t head,std::stop_token token)const{std::array<std::uint8_t,Constants::MinimumTrackHeaderLength> prefix{};source_->readExactly(offset,prefix,token);if(qFromLittleEndian<std::uint32_t>(prefix.data())!=Constants::TrackMarker||qFromLittleEndian<std::int32_t>(prefix.data()+Constants::TrackCylinderOffset)!=cylinder||qFromLittleEndian<std::int32_t>(prefix.data()+Constants::TrackHeadOffset)!=head)throw MediaError(DiagnosticCode::MfmTrack);}
void Container::readTrackBytes(std::int32_t cylinder,std::int32_t head,std::int32_t offset,MediaFileSystems::MutableBytes destination,std::stop_token token)const{
    checkCancellation(token);const auto track=header().trackOffset(cylinder,head);if(offset<0||offset>header().trackDataLength()||destination.size()>std::uint64_t(header().trackDataLength()-offset))throw MediaError(DiagnosticCode::MfmTrackRange);validateTrack(track,cylinder,head,token);const auto changed=changes_.find(track+header().trackHeaderLength());if(changed!=changes_.end())std::copy_n(changed->second->begin()+offset,destination.size(),destination.begin());else source_->readExactly(track+header().trackHeaderLength()+offset,destination,token);
}
void Container::copyTo(const ImageOutput& output,std::stop_token token)const{
    checkCancellation(token);if(!output)throw MediaError(DiagnosticCode::MfmOutput);for(std::int32_t c=0;c<header().cylinders();++c)for(std::int32_t h=0;h<header().heads();++h)validateTrack(header().trackOffset(c,h),c,h,token);
    std::vector<std::uint8_t> buffer(Constants::CopyBufferLength);const auto copy=[&](std::uint64_t offset,std::uint64_t length){while(length){checkCancellation(token);const auto count=std::min(std::uint64_t(buffer.size()),length);auto bytes=std::span(buffer).first(std::size_t(count));source_->readExactly(offset,bytes,token);output(bytes);offset+=count;length-=count;}};
    std::uint64_t offset{};for(const auto& [position,bytes]:changes_){copy(offset,position-offset);checkCancellation(token);output(*bytes);offset=position+bytes->size();}copy(offset,source_->length()-offset);
}
}
