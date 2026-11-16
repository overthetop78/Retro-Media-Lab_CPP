#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <array>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine::VocTape {
namespace {
using namespace Constants;
bool select(const MediaDocument& document) {
    int channels=0,rate=0,bits=0,align=0;if(!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))return false;
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments()||image->segments()->empty())return false;bool found=false;
    for(const auto& segment:*image->segments()){const auto& value=segment.fields();if(value.kind!=SequentialSegmentKind::Samples||(value.channelNumber&&*value.channelNumber!=0))continue;if(!value.dataRange||value.dataRange->kind()!=DataRangeKind::Stored||!value.dataRange->source()||value.dataRange->length()%align)return false;found=true;}return found;
}
}
const WriterCapabilities& Writer::capabilities()const {
    static const WriterCapabilities value{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return value;
}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const {
    return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&select(document);
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const {
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token] {
        checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::VocWrite);
        const auto extension=path.extension().u8string();if(!Writer{}.canWrite(*document,format,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::VocWrite);
        int channels=0,sampleRate=0,bits=0,align=0;TapePcm::tryProfile(document->metadata(),channels,sampleRate,bits,align);
        std::vector<std::int32_t> samples;const auto& image=dynamic_cast<const SequentialImage&>(document->representation());
        for(const auto& segment:*image.segments()) {
            const auto& value=segment.fields();if(value.kind!=SequentialSegmentKind::Samples||(value.channelNumber&&*value.channelNumber!=0))continue;
            const auto bytes=readStoredRange(&*value.dataRange,token);const auto mono=TapePcm::extractChannel(*bytes,channels,bits,align,0);
            if(mono.size()>std::size_t(TapePcm::Constants::MaximumBuffer)-samples.size())throw MediaError(DiagnosticCode::VocWrite);
            samples.insert(samples.end(),mono.begin(),mono.end());
        }
        const auto timeConstant=std::uint8_t(std::clamp(TapePcm::roundEven(RateBase-double(RateNumerator)/sampleRate),double(1),double(MaximumByte)));
        const auto rate=double(RateNumerator)/(RateBase-timeConstant);const auto size=TapePcm::roundEven(double(samples.size())*rate/sampleRate);
        if(!std::isfinite(size)||size>TapePcm::Constants::MaximumBuffer)throw MediaError(DiagnosticCode::VocWrite);
        auto pcm=std::vector<std::uint8_t>(std::size_t(size));const auto shift=bits-ByteBits;
        for(std::size_t i=0;i<pcm.size();++i) {
            checkCancellation(token);const auto index=std::min(samples.size()-1,std::size_t(std::floor(double(i)*sampleRate/rate)));
            const auto value=samples[index]>>shift;pcm[i]=std::uint8_t(std::clamp(value+SampleBias,0,MaximumByte));
        }
        writeAtomicImage(path,[&](const auto& output,auto stop) {
            std::array<std::uint8_t,HeaderSize> header{};std::copy(Signature.begin(),Signature.end(),header.begin());qToLittleEndian(std::uint16_t(HeaderSize),header.data()+DataOffset);qToLittleEndian(Version,header.data()+VersionOffset);qToLittleEndian(std::uint16_t(~Version+ChecksumBase),header.data()+ChecksumOffset);output(header);
            std::size_t offset=0;
            do {
                checkCancellation(stop);const auto first=offset==0;const auto count=std::min(pcm.size()-offset,std::size_t(first?MaximumFirstBlock:MaximumBlock));const auto length=std::uint32_t(count+(first?SoundFields:0));std::array<std::uint8_t,BlockHeaderSize> block{};block[0]=first?Sound:Continuation;for(int i=0;i<LengthBytes;++i)block[i+1]=std::uint8_t(length>>(i*ByteBits));output(block);
                if(first){const std::array<std::uint8_t,SoundFields> fields{timeConstant,0};output(fields);}output(std::span(pcm).subspan(offset,count));offset+=count;
            }while(offset<pcm.size());
            const std::array<std::uint8_t,1> end{};output(end);
        },token);
        return std::vector<std::filesystem::path>{path};
    });
}
}
