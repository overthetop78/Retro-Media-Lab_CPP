#include "Reader.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <QtEndian>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::WavTape {
namespace {
using namespace Constants;
struct Chunk {std::string id;std::int64_t offset;std::uint32_t length;};
bool waveHeader(std::span<const std::uint8_t> bytes) {
    return bytes.size()>=HeaderSize && std::equal(Riff.begin(),Riff.end(),bytes.begin()) && std::equal(Wave.begin(),Wave.end(),bytes.begin()+WaveOffset);
}
bool supported(int channels,std::uint32_t rate,int bits) {
    return channels>=1 && channels<=MaximumChannels && rate>=1 && rate<=MaximumRate && std::find(TapePcm::Constants::SupportedBits.begin(),TapePcm::Constants::SupportedBits.end(),bits)!=TapePcm::Constants::SupportedBits.end();
}
SequentialTime time(std::uint64_t frames,std::uint32_t rate) {
    return SequentialTime(std::int64_t(double(frames)/rate*SequentialTicksPerSecond));
}
}
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities value{{std::string(FormatId)},{std::string(Extension)},{{Riff.begin(),Riff.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return value;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        checkCancellation(token);const auto requested=context.requestedFormatId();
        if((requested&&!sameIdentifier(*requested,FormatId))||context.length()<HeaderSize)return false;
        return waveHeader(context.readHeaderAsync(HeaderSize,token).get());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token] {
        const auto header=context.readHeaderAsync(HeaderSize,token).get();
        if(!waveHeader(header))throw MediaError(DiagnosticCode::WaveData);
        const auto riffLength=std::int64_t(qFromLittleEndian<std::uint32_t>(header.data()+LengthOffset))+ChunkHeaderSize;
        if(riffLength<HeaderSize||riffLength>context.length())throw MediaError(DiagnosticCode::WaveData);
        std::uint16_t channels=0,bits=0,align=0;std::uint32_t rate=0;std::vector<Chunk> chunks;
        for(std::int64_t position=HeaderSize;position<=riffLength-ChunkHeaderSize;) {
            const auto chunk=context.readAsync(position,ChunkHeaderSize,token).get();
            const std::string id(chunk.begin(),chunk.begin()+TagSize);const auto length=qFromLittleEndian<std::uint32_t>(chunk.data()+LengthOffset);const auto offset=position+ChunkHeaderSize;
            if(length>riffLength-offset)throw MediaError(DiagnosticCode::WaveData);
            if(id==Format) {
                if(length<FormatSize)throw MediaError(DiagnosticCode::WaveData);
                const auto fields=context.readAsync(offset,FormatSize,token).get();
                if(qFromLittleEndian<std::uint16_t>(fields.data())!=PcmTag)throw MediaError(DiagnosticCode::WaveTapeProfile);
                channels=qFromLittleEndian<std::uint16_t>(fields.data()+ChannelsOffset);rate=qFromLittleEndian<std::uint32_t>(fields.data()+RateOffset);align=qFromLittleEndian<std::uint16_t>(fields.data()+AlignOffset);bits=qFromLittleEndian<std::uint16_t>(fields.data()+BitsOffset);
            }
            chunks.push_back({id,offset,length});position=offset+length+(length&1u);
        }
        if(!supported(channels,rate,bits))throw MediaError(DiagnosticCode::WaveTapeProfile);
        if(align!=channels*(bits/TapePcm::Constants::ByteBits))throw MediaError(DiagnosticCode::WaveData);
        if(std::none_of(chunks.begin(),chunks.end(),[](const auto& chunk){return chunk.id==Data;}))throw MediaError(DiagnosticCode::WaveData);
        std::vector<SequentialSegment> segments;std::uint64_t totalFrames=0;const auto source=context.dataSource();
        for(const auto& chunk:chunks) {
            checkCancellation(token);
            if(chunk.id==Data) {
                if(chunk.length%align)throw MediaError(DiagnosticCode::WaveData);const auto frames=chunk.length/align;if(!frames)continue;
                for(int channel=0;channel<channels;++channel) {
                    SequentialSegmentFields value;value.position=std::int64_t(totalFrames);value.kind=SequentialSegmentKind::Samples;value.length=frames;value.start=time(totalFrames,rate);value.duration=time(frames,rate);value.channelNumber=channel;value.direction=SequentialTravelDirection::Forward;
                    value.dataRange=DataRange(chunk.offset,chunk.length,DataRangeKind::Stored,source,chunk.offset);
                    value.metadata={{std::string(ChannelKey),std::to_string(channel)},{std::string(InterleavedKey),std::string(TapePcm::Constants::TrueValue)},{std::string(TapePcm::Constants::AlignKey),std::to_string(align)}};
                    segments.emplace_back(std::move(value));
                }
                totalFrames+=frames;
            } else if(chunk.id!=Format) {
                SequentialSegmentFields value;value.position=chunk.offset;value.kind=SequentialSegmentKind::Unknown;
                if(chunk.length){value.length=chunk.length;value.dataRange=DataRange(chunk.offset,chunk.length,DataRangeKind::Stored,source,chunk.offset);}
                value.metadata={{std::string(ChunkKey),chunk.id}};segments.emplace_back(std::move(value));
            }
        }
        std::vector<MediaFileSystems::MessageRecipe> messages;
        if(riffLength!=context.length())messages.push_back({std::string(DiagnosticKeys::WaveTapeTrailing),{}, {}});
        TapePcm::Parameters metadata{{std::string(TapePcm::Constants::ChannelsKey),std::to_string(channels)},{std::string(TapePcm::Constants::RateKey),std::to_string(rate)},{std::string(TapePcm::Constants::BitsKey),std::to_string(bits)},{std::string(TapePcm::Constants::AlignKey),std::to_string(align)}};
        return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),time(totalFrames,rate),std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::move(messages),std::move(metadata));
    });
}
}
