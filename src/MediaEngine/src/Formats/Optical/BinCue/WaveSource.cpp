#include "WaveSource.h"
#include "WaveConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <array>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::BinCue {
namespace {
using namespace WaveConstants;
class WaveData final:public MediaFileSystems::RandomAccessData {
public:
    WaveData(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::uint64_t offset,std::uint64_t length):source_(std::move(source)),offset_(offset),length_(length){}
    std::uint64_t length()const noexcept override{return length_;}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{checkCancellation(token);if(offset>length_||output.size()>length_-offset)throw MediaError(DiagnosticCode::WaveRange);source_->readExactly(offset_+offset,output,token);}
private:std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;std::uint64_t offset_,length_;
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> wavePcmSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,bool requireSectors,std::stop_token token){
    using namespace WaveConstants;checkCancellation(token);if(!source||source->length()<RiffHeaderBytes)throw MediaError(DiagnosticCode::WaveData);std::array<std::uint8_t,RiffHeaderBytes> header{};source->readExactly(0,header,token);if(!std::equal(Riff.begin(),Riff.end(),header.begin())||!std::equal(Wave.begin(),Wave.end(),header.begin()+FormOffset))throw MediaError(DiagnosticCode::WaveData);
    bool hasFormat=false;std::uint64_t dataOffset=0,dataLength=0,position=RiffHeaderBytes;
    while(position<=source->length()&&ChunkHeaderBytes<=source->length()-position){checkCancellation(token);std::array<std::uint8_t,ChunkHeaderBytes> chunk{};source->readExactly(position,chunk,token);const auto length=qFromLittleEndian<std::uint32_t>(chunk.data()+ChunkLengthOffset);const auto start=position+ChunkHeaderBytes;if(length>source->length()-start)throw MediaError(DiagnosticCode::WaveData);
        if(std::equal(Format.begin(),Format.end(),chunk.begin())){if(length<FormatBytes)throw MediaError(DiagnosticCode::WaveData);std::array<std::uint8_t,FormatBytes> fields{};source->readExactly(start,fields,token);
            if(qFromLittleEndian<std::uint16_t>(fields.data())!=Pcm||qFromLittleEndian<std::uint16_t>(fields.data()+ChannelsOffset)!=Stereo||qFromLittleEndian<std::uint32_t>(fields.data()+RateOffset)!=SampleRate||qFromLittleEndian<std::uint32_t>(fields.data()+ByteRateOffset)!=ByteRate||qFromLittleEndian<std::uint16_t>(fields.data()+AlignOffset)!=BlockAlign||qFromLittleEndian<std::uint16_t>(fields.data()+BitsOffset)!=SampleBits)throw MediaError(DiagnosticCode::WaveProfile);hasFormat=true;}
        else if(std::equal(Data.begin(),Data.end(),chunk.begin())){dataOffset=start;dataLength=length;}
        const auto padding=length&1u;if(padding>source->length()-start-length)throw MediaError(DiagnosticCode::WaveData);position=start+length+padding;
    }
    if(!hasFormat||!dataLength||dataLength%BlockAlign||(requireSectors&&dataLength%AudioSectorBytes))throw MediaError(DiagnosticCode::WaveData);return std::make_shared<WaveData>(std::move(source),dataOffset,dataLength);
}
}
