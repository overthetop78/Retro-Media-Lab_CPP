#include "Flac.h"
#include "CodecConstants.h"
#include "../../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <bit>
#include <limits>
namespace RetroMediaLab::MediaEngine::Chd {
namespace {
std::int64_t shiftLeft(std::int64_t value,int bits){return std::bit_cast<std::int64_t>(std::uint64_t(value)<<bits);}
std::int64_t add(std::int64_t left,std::int64_t right){return std::bit_cast<std::int64_t>(std::uint64_t(left)+std::uint64_t(right));}
std::int64_t subtract(std::int64_t left,std::int64_t right){return std::bit_cast<std::int64_t>(std::uint64_t(left)-std::uint64_t(right));}
class FlacBits final {
public:
    FlacBits(std::span<const std::uint8_t> source,std::stop_token token):source_(source),token_(token){}
    std::size_t offset()const noexcept{return offset_-std::size_t(count_/CodecConstants::ByteBits);}
    std::uint64_t read(int width){using namespace CodecConstants;if(width<0||width>FlacMaximumWidth)throw MediaError(DiagnosticCode::ChdFlacBitWidth);while(count_<width){if(offset_>=source_.size())throw MediaError(DiagnosticCode::ChdFlacTruncated);if(!(offset_&PollMask))checkCancellation(token_);buffer_=(buffer_<<ByteBits)|source_[offset_++];count_+=ByteBits;}count_-=width;return (buffer_>>count_)&((std::uint64_t(1)<<width)-1);}
    std::int64_t signedValue(int width){if(!width)return 0;const auto value=read(width),sign=std::uint64_t(1)<<(width-1);return std::bit_cast<std::int64_t>((value^sign)-sign);}
    std::uint32_t unary(){std::uint32_t count=0;while(!read(1)){if(count==std::numeric_limits<std::uint32_t>::max())throw MediaError(DiagnosticCode::ChdFlacUnary);++count;}return count;}
    void align(){read(count_&(CodecConstants::ByteBits-1));}
private:std::span<const std::uint8_t> source_;std::stop_token token_;std::uint64_t buffer_=0;int count_=0;std::size_t offset_=0;
};
std::uint8_t headerCrc(std::span<const std::uint8_t> bytes){using namespace CodecConstants;int value=0;for(auto byte:bytes){value^=byte;for(int bit=0;bit<ByteBits;++bit)value=((value<<1)^((value&FlacHeaderHigh)?FlacHeaderPolynomial:0))&ByteMask;}return std::uint8_t(value);}
void decodeSubframe(FlacBits& bits,std::span<std::int64_t> samples,int sampleBits,std::stop_token token){
    using namespace CodecConstants;if(bits.read(1))throw MediaError(DiagnosticCode::ChdFlacSubframe);const auto type=int(bits.read(FlacSubframeTypeBits));std::uint32_t wasted=0;if(bits.read(1)){const auto unary=bits.unary();if(unary>=std::uint32_t(sampleBits))throw MediaError(DiagnosticCode::ChdFlacSubframe);wasted=unary+1;}const auto width=sampleBits-int(wasted);if(width<=0)throw MediaError(DiagnosticCode::ChdFlacSubframe);
    if(!type)std::fill(samples.begin(),samples.end(),bits.signedValue(width));
    else if(type==1){for(std::size_t index=0;index<samples.size();++index){if(!(index&PollMask))checkCancellation(token);samples[index]=bits.signedValue(width);}}
    else {
        const auto fixed=type>=FlacFixedFirst&&type<=FlacFixedLast;if(!fixed&&type<FlacLpcFirst)throw MediaError(DiagnosticCode::ChdFlacSubframe);const auto order=fixed?type-FlacFixedFirst:type-FlacLpcOrderBias;if(std::size_t(order)>samples.size())throw MediaError(DiagnosticCode::ChdFlacPredictor);
        for(int index=0;index<order;++index)samples[std::size_t(index)]=bits.signedValue(width);std::array<int,FlacMaximumOrder> coefficients{};int shift=0;
        if(fixed)std::copy_n(FlacFixedCoefficients[std::size_t(order)].begin(),order,coefficients.begin());
        else{const auto precision=int(bits.read(FlacPrecisionBits))+1;if(precision==FlacReservedPrecision)throw MediaError(DiagnosticCode::ChdFlacPredictor);shift=int(bits.signedValue(FlacShiftBits));for(int index=0;index<order;++index)coefficients[std::size_t(index)]=int(bits.signedValue(precision));}
        const auto method=int(bits.read(FlacResidualMethodBits)),partitions=int(bits.read(FlacPartitionBits));if(method>1||(samples.size()&((std::size_t(1)<<partitions)-1)))throw MediaError(DiagnosticCode::ChdFlacResidual);const auto partitionSize=samples.size()>>partitions;if(partitionSize<std::size_t(order))throw MediaError(DiagnosticCode::ChdFlacResidual);auto position=std::size_t(order);const auto riceWidth=method?FlacRice2Width:FlacRiceWidth;
        for(std::size_t partition=0;partition<(std::size_t(1)<<partitions);++partition){checkCancellation(token);const auto rice=int(bits.read(riceWidth)),rawWidth=rice==((1<<riceWidth)-1)?int(bits.read(FlacRawWidthBits)):-1;const auto count=partitionSize-(partition?0:std::size_t(order));
            for(std::size_t index=0;index<count;++index){std::int64_t residual;if(rawWidth>=0)residual=bits.signedValue(rawWidth);else{const auto unary=bits.unary();const auto folded=(std::uint64_t(unary)<<rice)|bits.read(rice);residual=std::bit_cast<std::int64_t>((folded>>1)^std::uint64_t(-std::int64_t(folded&1)));}
                std::uint64_t accumulated=0;for(int coefficient=0;coefficient<order;++coefficient)accumulated+=std::uint64_t(std::int64_t(coefficients[std::size_t(coefficient)]))*std::uint64_t(samples[position-std::size_t(coefficient)-1]);auto prediction=std::bit_cast<std::int64_t>(accumulated);prediction=shift>=0?prediction>>shift:shiftLeft(prediction,-shift);samples[position++]=add(prediction,residual);
            }
        }
    }
    if(wasted)for(auto& sample:samples)sample=shiftLeft(sample,int(wasted));
}
}
std::size_t Flac::decodeFrames(std::span<const std::uint8_t> source,std::span<std::uint8_t> destination,bool bigEndian,std::stop_token token){
    using namespace CodecConstants;checkCancellation(token);if(destination.size()%FlacStereoBytes||destination.size()>MaximumHunk)throw MediaError(DiagnosticCode::ChdFlacSamples);FlacBits bits(source,token);std::size_t written=0;
    while(written<destination.size()){
        checkCancellation(token);const auto frameStart=bits.offset();if(bits.read(FlacSyncBits)!=FlacSync||bits.read(1))throw MediaError(DiagnosticCode::ChdFlacFrame);bits.read(1);const auto blockCode=int(bits.read(FlacCodeBits)),rateCode=int(bits.read(FlacCodeBits)),channelCode=int(bits.read(FlacCodeBits)),sizeCode=int(bits.read(FlacSizeCodeBits));
        if(bits.read(1)||rateCode==FlacReservedRate||(channelCode!=FlacIndependent&&channelCode!=FlacLeftSide&&channelCode!=FlacRightSide&&channelCode!=FlacMidSide))throw MediaError(DiagnosticCode::ChdFlacFrame);const auto sampleBits=FlacSampleBits[std::size_t(sizeCode)];if(!sampleBits)throw MediaError(DiagnosticCode::ChdFlacFrame);
        const auto first=int(bits.read(ByteBits));int extra=0;if(first&FlacFrameNumberHigh){int mask=FlacFrameNumberHigh;while(mask&&(first&mask)){++extra;mask>>=1;}if(extra<FlacFrameNumberMinimum||extra>FlacFrameNumberMaximum)throw MediaError(DiagnosticCode::ChdFlacFrame);for(int index=1;index<extra;++index)bits.read(ByteBits);}
        int block=0;if(blockCode==1)block=FlacShortBlockBase;else if(blockCode>=FlacMiddleBlockFirst&&blockCode<=FlacMiddleBlockLast)block=FlacMiddleBlockBase<<(blockCode-FlacMiddleBlockFirst);else if(blockCode==FlacByteBlock)block=int(bits.read(ByteBits))+1;else if(blockCode==FlacWordBlock)block=int(bits.read(FlacDefaultSampleBits))+1;else if(blockCode>=FlacLongBlockFirst)block=FlacLongBlockBase<<(blockCode-FlacLongBlockFirst);if(!block)throw MediaError(DiagnosticCode::ChdFlacFrame);
        if(rateCode==FlacByteRate)bits.read(ByteBits);else if(rateCode==FlacWordRate1||rateCode==FlacWordRate2)bits.read(FlacDefaultSampleBits);const auto headerEnd=bits.offset();const auto expectedHeader=bits.read(ByteBits);if(headerCrc(source.subspan(frameStart,headerEnd-frameStart))!=expectedHeader)throw MediaError(DiagnosticCode::ChdFlacHeaderCrc);
        if(left_.size()<std::size_t(block)){left_.resize(std::size_t(block));right_.resize(std::size_t(block));}decodeSubframe(bits,std::span(left_).first(std::size_t(block)),sampleBits+(channelCode==FlacRightSide),token);decodeSubframe(bits,std::span(right_).first(std::size_t(block)),sampleBits+(channelCode==FlacLeftSide||channelCode==FlacMidSide),token);bits.align();const auto frameEnd=bits.offset();const auto expectedFrame=bits.read(FlacDefaultSampleBits);if(Crc16::compute(source.subspan(frameStart,frameEnd-frameStart),FlacFramePolynomial,0,token)!=expectedFrame)throw MediaError(DiagnosticCode::ChdFlacFrameCrc);
        const auto frames=std::min(std::size_t(block),(destination.size()-written)/FlacStereoBytes);
        for(std::size_t index=0;index<frames;++index){auto left=left_[index],right=right_[index];if(channelCode==FlacLeftSide)right=subtract(left,right);else if(channelCode==FlacRightSide)left=add(left,right);else if(channelCode==FlacMidSide){const auto mid=std::bit_cast<std::int64_t>(std::uint64_t(shiftLeft(left,1))|(std::uint64_t(right)&1));left=add(mid,right)>>1;right=subtract(mid,right)>>1;}
            const auto l=std::uint16_t(sampleBits<=FlacDefaultSampleBits?shiftLeft(left,FlacDefaultSampleBits-sampleBits):left>>(sampleBits-FlacDefaultSampleBits)),r=std::uint16_t(sampleBits<=FlacDefaultSampleBits?shiftLeft(right,FlacDefaultSampleBits-sampleBits):right>>(sampleBits-FlacDefaultSampleBits));
            if(bigEndian){qToBigEndian(l,destination.data()+written);qToBigEndian(r,destination.data()+written+FlacSampleBytes);}else{qToLittleEndian(l,destination.data()+written);qToLittleEndian(r,destination.data()+written+FlacSampleBytes);}written+=FlacStereoBytes;
        }
    }return bits.offset();
}
}
