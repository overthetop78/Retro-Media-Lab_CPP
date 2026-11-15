#include "Writer.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaEngine::Scp {
namespace {
using namespace Constants;
using Bytes=MediaFileSystems::Bytes;
using MutableBytes=MediaFileSystems::MutableBytes;
struct RevolutionPlan {std::uint32_t words=0,checksum=0;};
struct TrackPlan {const Track* source=nullptr;std::vector<std::uint8_t> descriptors;std::vector<RevolutionPlan> revolutions;std::uint64_t length=0;std::uint32_t checksum=0;};
struct WritePlan {std::array<std::uint8_t,HeaderBytes> header{};std::array<std::uint8_t,TableBytes> table{};std::vector<TrackPlan> tracks;};
void putLittle(MutableBytes data,std::size_t offset,std::uint32_t value) {
    for(std::uint32_t byte=0;byte<FieldBytes;++byte)data[offset+byte]=std::uint8_t(value>>(byte*std::numeric_limits<std::uint8_t>::digits));
}
std::array<std::uint8_t,HeaderBytes> buildHeader(const Header& h) {
    std::array<std::uint8_t,HeaderBytes> bytes{};std::copy(Signature.begin(),Signature.end(),bytes.begin());
    bytes[VersionOffset]=h.version;bytes[DiskTypeOffset]=h.diskType;bytes[RevolutionsOffset]=h.revolutions;bytes[StartOffset]=h.startTrack;bytes[EndOffset]=h.endTrack;
    bytes[FlagsOffset]=h.flags;bytes[WidthOffset]=h.bitCellEncoding;bytes[HeadsOffset]=h.heads;bytes[ResolutionOffset]=h.resolution;
    return bytes;
}
void validate(const Image& image,std::stop_token token) {
    const auto& h=image.header;
    (void)readHeader(buildHeader(h));
    if(h.flags&ExtendedFlag)throw MediaError(DiagnosticCode::ScpExtended);
    std::set<std::uint8_t> seen;
    for(const auto& track:image.tracks) {
        checkCancellation(token);
        if(!seen.insert(track.trackNumber).second)throw MediaError(DiagnosticCode::ScpDuplicateTrack,{std::to_string(track.trackNumber)});
        if(track.trackNumber<h.startTrack||track.trackNumber>h.endTrack)throw MediaError(DiagnosticCode::ScpTrackOutside,{std::to_string(track.trackNumber),std::to_string(h.startTrack),std::to_string(h.endTrack)});
        const auto [cylinder,head]=trackAddress(track.trackNumber);
        if(track.cylinder!=cylinder||track.head!=head)throw MediaError(DiagnosticCode::ScpWriteAddress,{std::to_string(track.trackNumber),std::to_string(track.cylinder),std::to_string(track.head)});
        if(track.revolutions.size()!=h.revolutions)throw MediaError(DiagnosticCode::ScpWriteRevolutions,{std::to_string(track.trackNumber),std::to_string(track.revolutions.size()),std::to_string(h.revolutions)});
        for(std::size_t index=0;index<track.revolutions.size();++index) {
            for(const auto interval:track.revolutions[index].flux.intervals) {
                checkCancellation(token);
                if(!interval)throw MediaError(DiagnosticCode::ScpZeroInterval,{std::to_string(track.trackNumber),std::to_string(index+1)});
            }
        }
    }
}
// Runs avoid constructing a word array for large intervals. The same walk plans and writes the bytes.
template<class Emit> void encodedWords(const Revolution& revolution,Emit emit,std::stop_token token) {
    std::uint64_t carry=0;
    for(std::size_t index=0;index<revolution.flux.intervals.size();++index) {
        checkCancellation(token);
        auto remaining=std::uint64_t(revolution.flux.intervals[index])+carry;carry=0;
        if(remaining>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
        if(index+1<revolution.flux.intervals.size()&&remaining%OverflowTicks==0){--remaining;carry=1;}
        const auto zeros=remaining/OverflowTicks,tail=remaining%OverflowTicks;
        if(zeros)emit(std::uint16_t(0),zeros);
        if(tail)emit(std::uint16_t(tail),std::uint64_t(1));
    }
}
TrackPlan planTrack(const Track& track,std::stop_token token) {
    TrackPlan result;result.source=&track;
    const auto descriptorLength=TrackHeaderBytes+track.revolutions.size()*RevolutionDescriptorBytes;
    if(descriptorLength>MaximumLength)throw MediaError(DiagnosticCode::ScpOverflow);
    result.descriptors.resize(descriptorLength);
    std::copy(TrackSignature.begin(),TrackSignature.end(),result.descriptors.begin());result.descriptors[TrackNumberOffset]=track.trackNumber;
    result.length=descriptorLength;
    for(std::size_t index=0;index<track.revolutions.size();++index) {
        RevolutionPlan revolution;
        encodedWords(track.revolutions[index],[&](std::uint16_t word,std::uint64_t count){
            if(count>(MaximumLength-result.length)/WordBytes)throw MediaError(DiagnosticCode::ScpOverflow);
            result.length+=count*WordBytes;revolution.words+=std::uint32_t(count);
            const auto byteSum=std::uint32_t(word/std::uint32_t(std::numeric_limits<std::uint8_t>::max()+1))+std::uint8_t(word);
            revolution.checksum+=std::uint32_t(std::uint64_t(byteSum)*count);
        },token);
        result.revolutions.push_back(revolution);
    }
    std::uint32_t offset=std::uint32_t(descriptorLength);
    for(std::size_t index=0;index<result.revolutions.size();++index) {
        const auto descriptor=TrackHeaderBytes+index*RevolutionDescriptorBytes;
        putLittle(result.descriptors,descriptor,track.revolutions[index].flux.indexTimeTicks);
        putLittle(result.descriptors,descriptor+CountOffset,result.revolutions[index].words);
        putLittle(result.descriptors,descriptor+DataOffset,offset);
        offset+=result.revolutions[index].words*WordBytes;
        result.checksum+=result.revolutions[index].checksum;
    }
    result.checksum=updateChecksum(result.checksum,result.descriptors);
    return result;
}
WritePlan plan(const Image& image,std::stop_token token) {
    checkCancellation(token);validate(image,token);
    WritePlan result;result.header=buildHeader(image.header);
    std::vector<const Track*> tracks;tracks.reserve(image.tracks.size());for(const auto& track:image.tracks)tracks.push_back(&track);
    std::sort(tracks.begin(),tracks.end(),[](auto a,auto b){return a->trackNumber<b->trackNumber;});
    std::uint64_t offset=FullHeaderBytes;std::uint32_t checksum=0;
    for(const auto* track:tracks) {
        if(offset>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
        putLittle(result.table,std::size_t(track->trackNumber)*FieldBytes,std::uint32_t(offset));
        auto entry=planTrack(*track,token);offset+=entry.length;checksum+=entry.checksum;result.tracks.push_back(std::move(entry));
    }
    checksum=updateChecksum(checksum,result.table);putLittle(result.header,ChecksumOffset,checksum);
    return result;
}
void outputPlan(const WritePlan& plan,const ImageOutput& output,std::stop_token token) {
    if(!output)throw MediaError(DiagnosticCode::ScpOutput);
    checkCancellation(token);output(plan.header);checkCancellation(token);output(plan.table);
    std::array<std::uint8_t,ChunkBytes> buffer{};std::size_t used=0;
    const auto flush=[&]{if(used){checkCancellation(token);output(Bytes(buffer).first(used));used=0;}};
    for(const auto& track:plan.tracks) {
        checkCancellation(token);output(track.descriptors);
        for(const auto& revolution:track.source->revolutions) {
            encodedWords(revolution,[&](std::uint16_t word,std::uint64_t count){
                for(std::uint64_t index=0;index<count;++index) {
                    buffer[used++]=std::uint8_t(word/std::uint32_t(std::numeric_limits<std::uint8_t>::max()+1));buffer[used++]=std::uint8_t(word);
                    if(used==buffer.size())flush();
                }
            },token);
        }
        flush();
    }
    checkCancellation(token);
}
}
void write(const Image& image,const ImageOutput& output,std::stop_token token) {const auto prepared=plan(image,token);outputPlan(prepared,output,token);}
void write(const Image& image,const std::filesystem::path& path,std::stop_token token) {
    const auto prepared=plan(image,token);
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token active){outputPlan(prepared,output,active);},token);
}
}
