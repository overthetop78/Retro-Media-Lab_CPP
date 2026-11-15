#include "Scp.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
#include <limits>
namespace RetroMediaLab::MediaEngine::Scp {
namespace {
using namespace Constants;
using MediaFileSystems::Bytes;
using MediaFileSystems::RandomAccessData;
std::uint32_t little(Bytes data,std::size_t offset) {
    std::uint32_t result=0;
    for(std::uint32_t byte=0;byte<FieldBytes;++byte)result|=std::uint32_t(data[offset+byte])<<(byte*std::numeric_limits<std::uint8_t>::digits);
    return result;
}
void require(std::uint64_t available,std::uint64_t offset,std::uint64_t length,DiagnosticCode code,std::vector<std::string> context={}) {
    if(offset<=available&&length<=available-offset)return;
    context.push_back(std::to_string(offset));context.push_back(std::to_string(length));
    throw MediaError(code,std::move(context));
}
void checkedRange(std::uint64_t offset,std::uint64_t count) {
    if(offset>MaximumLength||count>MaximumLength)throw MediaError(DiagnosticCode::ScpOverflow);
}
std::vector<std::uint8_t> readPart(const RandomAccessData& source,std::uint64_t offset,std::uint64_t count,DiagnosticCode code,std::vector<std::string> context,std::stop_token token) {
    checkCancellation(token);checkedRange(offset,count);
    require(source.length(),offset,count,code,std::move(context));
    std::vector<std::uint8_t> result(static_cast<std::size_t>(count));
    source.readExactly(offset,result,token);
    return result;
}
class BorrowedBytes final:public RandomAccessData {
public:
    explicit BorrowedBytes(Bytes bytes):bytes_(bytes){}
    std::uint64_t length() const noexcept override{return bytes_.size();}
    void readExactly(std::uint64_t offset,MediaFileSystems::MutableBytes target,std::stop_token token) const override {
        checkCancellation(token);
        require(bytes_.size(),offset,target.size(),DiagnosticCode::ScpHeaderSection);
        std::copy_n(bytes_.begin()+std::size_t(offset),target.size(),target.begin());
    }
private:Bytes bytes_;
};
Revolution readRevolution(const RandomAccessData& source,std::uint32_t trackOffset,Bytes descriptor,std::uint32_t track,std::uint32_t index,std::stop_token token) {
    const auto ticks=little(descriptor,0),count=little(descriptor,CountOffset),relative=little(descriptor,DataOffset);
    const auto offset=std::uint64_t(trackOffset)+relative,length=std::uint64_t(count)*WordBytes;
    checkedRange(relative,length);checkedRange(offset,length);
    const std::vector<std::string> context{std::to_string(track),std::to_string(index+1)};
    require(source.length(),offset,length,DiagnosticCode::ScpFluxSection,context);
    Revolution result{{ticks,{}},count};
    // Reserve only what the bounded input actually declares, rather than its entire container.
    result.flux.intervals.reserve(std::min(count,ChunkBytes/WordBytes));
    std::uint64_t overflow=0;
    std::array<std::uint8_t,ChunkBytes> buffer{};
    for(std::uint64_t done=0;done<length;) {
        checkCancellation(token);
        const auto batch=std::size_t(std::min<std::uint64_t>(buffer.size(),length-done));
        source.readExactly(offset+done,MediaFileSystems::MutableBytes(buffer).first(batch),token);
        for(std::size_t position=0;position<batch;position+=WordBytes) {
            const auto value=std::uint32_t(buffer[position])*std::uint32_t(std::numeric_limits<std::uint8_t>::max()+1)+buffer[position+1];
            if(!value) {
                overflow+=OverflowTicks;
                if(overflow>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
                continue;
            }
            if(overflow+value>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
            result.flux.intervals.push_back(std::uint32_t(overflow+value));overflow=0;
        }
        done+=batch;
    }
    if(overflow)result.flux.intervals.push_back(std::uint32_t(overflow));
    return result;
}
}
bool hasSignature(MediaFileSystems::Bytes bytes) noexcept {
    return bytes.size()>=Constants::Signature.size()&&std::equal(Constants::Signature.begin(),Constants::Signature.end(),bytes.begin());
}
bool canRead(const MediaFileSystems::RandomAccessData& source,std::stop_token token) {
    checkCancellation(token);
    if(source.length()<Constants::HeaderBytes)return false;
    std::array<std::uint8_t,Constants::Signature.size()> signature{};
    source.readExactly(0,signature,token);return hasSignature(signature);
}
Header readHeader(MediaFileSystems::Bytes bytes) {
    using namespace Constants;
    require(bytes.size(),0,HeaderBytes,DiagnosticCode::ScpHeaderSection);
    if(!hasSignature(bytes))throw MediaError(DiagnosticCode::ScpSignature);
    const auto revolutions=bytes[RevolutionsOffset],start=bytes[StartOffset],end=bytes[EndOffset],width=bytes[WidthOffset],heads=bytes[HeadsOffset];
    if(revolutions<MinimumRevolutions||revolutions>MaximumRevolutions)throw MediaError(DiagnosticCode::ScpRevolutions,{std::to_string(revolutions)});
    if(end<start||end>=TrackSlots)throw MediaError(DiagnosticCode::ScpTrackRange,{std::to_string(start),std::to_string(end)});
    if(width&&width!=ExplicitWidth)throw MediaError(DiagnosticCode::ScpWidth,{std::to_string(width)});
    if(heads>MaximumHeads)throw MediaError(DiagnosticCode::ScpHeads,{std::to_string(heads)});
    return {bytes[VersionOffset],bytes[DiskTypeOffset],revolutions,start,end,bytes[FlagsOffset],width,heads,bytes[ResolutionOffset],little(bytes,ChecksumOffset)};
}
Image read(MediaFileSystems::Bytes bytes,std::stop_token token) {return read(BorrowedBytes(bytes),token);}
Image read(const MediaFileSystems::RandomAccessData& source,std::stop_token token) {
    using namespace Constants;
    checkCancellation(token);
    checkedRange(source.length(),0);
    Image result;result.fileSize=source.length();
    result.header=readHeader(readPart(source,0,HeaderBytes,DiagnosticCode::ScpHeaderSection,{},token));
    if(result.header.flags&ExtendedFlag)throw MediaError(DiagnosticCode::ScpExtended);
    require(source.length(),0,FullHeaderBytes,DiagnosticCode::ScpTableSection);
    const auto table=readPart(source,HeaderBytes,TableBytes,DiagnosticCode::ScpTableSection,{},token);
    for(std::uint32_t slot=result.header.startTrack;slot<=result.header.endTrack;++slot) {
        checkCancellation(token);
        const auto offset=little(table,slot*FieldBytes);
        if(!offset)continue;
        const auto size=TrackHeaderBytes+result.header.revolutions*RevolutionDescriptorBytes;
        const auto data=readPart(source,offset,size,DiagnosticCode::ScpTrackSection,{std::to_string(slot)},token);
        const auto observed=data[TrackNumberOffset];
        if(!std::equal(TrackSignature.begin(),TrackSignature.end(),data.begin()))throw MediaError(DiagnosticCode::ScpTrackSignature,{std::to_string(slot),std::to_string(observed)});
        if(observed!=slot)throw MediaError(DiagnosticCode::ScpTrackMismatch,{std::to_string(slot),std::to_string(observed)});
        const auto [cylinder,head]=trackAddress(std::int32_t(slot));
        Track track{std::uint8_t(slot),cylinder,head,{}};
        for(std::uint32_t index=0;index<result.header.revolutions;++index) {
            track.revolutions.push_back(readRevolution(source,offset,MediaFileSystems::Bytes(data).subspan(TrackHeaderBytes+index*RevolutionDescriptorBytes,RevolutionDescriptorBytes),slot,index,token));
        }
        result.tracks.push_back(std::move(track));
    }
    std::uint32_t checksum=0;
    std::array<std::uint8_t,ChunkBytes> buffer{};
    for(std::uint64_t offset=HeaderBytes;offset<source.length();) {
        checkCancellation(token);
        const auto count=std::size_t(std::min<std::uint64_t>(buffer.size(),source.length()-offset));
        source.readExactly(offset,MediaFileSystems::MutableBytes(buffer).first(count),token);
        checksum=updateChecksum(checksum,MediaFileSystems::Bytes(buffer).first(count));offset+=count;
    }
    result.checksumValid=isChecksumValid(result.header.checksum,result.header.flags,checksum);
    return result;
}
}
