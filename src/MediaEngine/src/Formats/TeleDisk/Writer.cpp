#include "Writer.h"
#include "Internal.h"
#include "Constants.h"
#include "SectorCodec.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <map>
namespace RetroMediaLab::MediaEngine::TeleDisk {
namespace M=MediaFileSystems;
std::uint8_t checkedByte(std::int64_t value){if(value<0||value>std::numeric_limits<std::uint8_t>::max())throw MediaError(DiagnosticCode::Td0ByteRange,{std::to_string(value)});return std::uint8_t(value);}
namespace {
using namespace Constants;
void word(std::vector<std::uint8_t>& bytes,std::size_t offset,std::uint16_t value){bytes[offset]=std::uint8_t(value);bytes[offset+1]=std::uint8_t(value>>BitsPerByte);}
void append(std::vector<std::uint8_t>& out,M::Bytes bytes){out.insert(out.end(),bytes.begin(),bytes.end());}
std::optional<std::uint8_t> sizeCode(std::uint32_t size){for(std::uint8_t code=0;code<=MaximumSectorSizeCode;++code)if((BaseSectorSize<<code)==size)return code;return {};}
void header(std::vector<std::uint8_t>& out,const Header& h,bool comment){std::vector<std::uint8_t> bytes(HeaderSize);std::copy(Signature.begin(),Signature.end(),bytes.begin());bytes[SequenceOffset]=h.sequence;bytes[CheckSignatureOffset]=h.checkSignature;bytes[VersionOffset]=h.version;bytes[DataRateOffset]=h.dataRate;bytes[DriveTypeOffset]=h.driveType;bytes[SteppingOffset]=std::uint8_t(comment?h.trackDensity|CommentPresentMask:h.trackDensity&~CommentPresentMask);bytes[DosModeOffset]=h.dosMode;bytes[SurfaceCountOffset]=h.surfaces;word(bytes,HeaderCrcOffset,crc(M::Bytes(bytes).first(HeaderCrcOffset)));append(out,bytes);}
void comment(std::vector<std::uint8_t>& out,const Comment& c){
    if(c.data.size()>std::numeric_limits<std::uint16_t>::max())throw MediaError(DiagnosticCode::Td0CommentLength);
    std::vector<std::uint8_t> bytes(CommentHeaderSize);word(bytes,CommentLengthOffset,std::uint16_t(c.data.size()));bytes[CommentYearOffset]=c.year;bytes[CommentMonthOffset]=c.month;bytes[CommentDayOffset]=c.day;bytes[CommentHourOffset]=c.hour;bytes[CommentMinuteOffset]=c.minute;bytes[CommentSecondOffset]=c.second;word(bytes,CommentCrcOffset,crc(c.data,crc(M::Bytes(bytes).subspan(CommentLengthOffset))));append(out,bytes);append(out,c.data);
}
void sector(std::vector<std::uint8_t>& out,const Sector& s){
    auto address=std::vector<std::string>{std::to_string(s.cylinder),std::to_string(s.head),std::to_string(s.number)};
    if(s.sizeCode>MaximumSectorSizeCode){address.push_back(std::to_string(s.sizeCode));throw MediaError(DiagnosticCode::Td0SizeCode,address);}
    const auto expected=BaseSectorSize<<s.sizeCode;const bool unavailable=bool(s.flags&UnavailableMask);
    if(unavailable!=!s.data)throw MediaError(DiagnosticCode::Td0Availability,address);
    if(s.data&&s.data->size()!=expected){address.push_back(std::to_string(s.data->size()));address.push_back(std::to_string(expected));throw MediaError(DiagnosticCode::Td0WriteSectorLength,address);}
    out.insert(out.end(),{s.cylinder,s.head,s.number,s.sizeCode,s.flags,s.data?std::uint8_t(crc(*s.data)):std::uint8_t(CrcInitial)});
    if(!s.data)return;const auto encoded=encodeSector(*s.data);const auto length=std::uint16_t(encoded.payload.size()+EncodingFieldSize);out.push_back(std::uint8_t(length));out.push_back(std::uint8_t(length>>BitsPerByte));out.push_back(std::uint8_t(encoded.encoding));append(out,encoded.payload);
}
Image detailed(const M::SectorImage& image,std::stop_token token){
    const auto code=sizeCode(image.geometry().blockSize);if(!code)throw MediaError(DiagnosticCode::Td0BlockSize,{std::to_string(image.geometry().blockSize)});
    std::map<std::pair<std::int64_t,std::int64_t>,std::vector<std::shared_ptr<const M::SectorBlock>>> groups;
    image.visitAvailableBlocks([&](auto logical){M::checkCancellation(token);const auto block=M::requireSectorBlock(image,logical,token);groups[{block->address.cylinder,block->address.head}].push_back(block);return true;},token);
    Image result;const auto& g=image.geometry();
    for(std::uint32_t cylinder=0;cylinder<g.cylinders;++cylinder)for(std::uint32_t head=0;head<g.heads;++head){
        M::checkCancellation(token);Track track{checkedByte(cylinder),checkedByte(head),{}};
        auto& blocks=groups[{cylinder,head}];std::stable_sort(blocks.begin(),blocks.end(),[](const auto& a,const auto& b){return a->address.number<b->address.number;});
        for(const auto& block:blocks){const auto recorded=block->recordedAddress.value_or(block->address);const auto flags=std::uint8_t((block->integrityValid==false||block->dataIntegrityValid==false?DataCrcError:0)|(block->deletedDataMark==true?DeletedData:0));track.sectors.push_back({checkedByte(recorded.cylinder),checkedByte(recorded.head),checkedByte(recorded.number),block->recordedSizeCode.value_or(*code),flags,block->data});}
        result.tracks.push_back(std::move(track));
    }
    result.header=createHeader(image,token);return result;
}
}
bool hasRepresentablePayload(const M::SectorImage& image,std::stop_token token){const auto defaultCode=sizeCode(image.geometry().blockSize);if(!defaultCode)return false;bool any=false,valid=true;image.visitAvailableBlocks([&](auto index){M::checkCancellation(token);const auto block=M::requireSectorBlock(image,index,token);any=true;const auto code=block->recordedSizeCode.value_or(*defaultCode);valid=code<=Constants::MaximumSectorSizeCode&&block->data.size()==(Constants::BaseSectorSize<<code);return valid;},token);return any&&valid;}
std::vector<std::uint8_t> build(const Image& image,std::stop_token token){
    using namespace Constants;M::checkCancellation(token);
    if(image.tracks.empty()||std::any_of(image.tracks.begin(),image.tracks.end(),[](const auto& track){return track.sectors.size()>=EndOfTracks;}))throw MediaError(DiagnosticCode::Td0TrackMap);
    std::vector<std::uint8_t> bytes;header(bytes,image.header,image.comment.has_value());if(image.comment)comment(bytes,*image.comment);
    for(const auto& track:image.tracks){M::checkCancellation(token);std::vector<std::uint8_t> h(TrackHeaderSize);h[TrackSectorCountOffset]=std::uint8_t(track.sectors.size());h[TrackCylinderOffset]=track.cylinder;h[TrackHeadOffset]=track.head;h[TrackCrcOffset]=std::uint8_t(crc(M::Bytes(h).first(TrackCrcOffset)));append(bytes,h);for(const auto& s:track.sectors){M::checkCancellation(token);sector(bytes,s);}}
    bytes.push_back(std::uint8_t(EndOfTracks));return bytes;
}
std::vector<std::uint8_t> build(const M::SectorImage& image,std::stop_token token){return build(detailed(image,token),token);}
void write(const Image& image,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(image,token);writeAtomicImage(path,[&](const ImageOutput& out,std::stop_token stop){M::checkCancellation(stop);out(bytes);},token);}
void write(const M::SectorImage& image,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(image,token);writeAtomicImage(path,[&](const ImageOutput& out,std::stop_token stop){M::checkCancellation(stop);out(bytes);},token);}
}
