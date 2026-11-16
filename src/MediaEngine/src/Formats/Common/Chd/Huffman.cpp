#include "Huffman.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Chd {
Huffman::Huffman(int symbols,int maximum):maximumBits_(maximum){if(symbols<=0||maximum<1||maximum>CodecConstants::HuffmanMaximumBits)throw MediaError(DiagnosticCode::ChdHuffmanParameters);lengths_.resize(std::size_t(symbols));lookup_.resize(std::size_t(1)<<maximum);}
int Huffman::decode(Bitstream& bits)const{using namespace CodecConstants;const auto entry=lookup_[bits.peek(maximumBits_)];const auto length=int(entry&HuffmanLengthMask);if(!length)throw MediaError(DiagnosticCode::ChdHuffmanCode);bits.remove(length);return int(entry>>HuffmanEntryShift);}
void Huffman::importRle(Bitstream& bits){
    using namespace CodecConstants;const auto width=maximumBits_>=HuffmanLongBits?HuffmanLongWidth:maximumBits_>=HuffmanMediumBits?HuffmanMediumWidth:HuffmanShortWidth;
    for(std::size_t node=0;node<lengths_.size();){auto length=bits.read(width);std::size_t repeat=1;if(length==1){length=bits.read(width);if(length!=1)repeat=bits.read(width)+HuffmanRunBias;}if(repeat>lengths_.size()-node)throw MediaError(DiagnosticCode::ChdHuffmanRun);std::fill_n(lengths_.begin()+node,repeat,std::uint8_t(length));node+=repeat;}
    build();if(bits.overflow())throw MediaError(DiagnosticCode::ChdHuffmanTruncated);
}
void Huffman::importHuffman(Bitstream& bits){
    using namespace CodecConstants;Huffman small(SmallSymbols,SmallMaximumBits);small.lengths_[0]=std::uint8_t(bits.read(SmallLengthBits));const auto start=bits.read(SmallLengthBits)+1;std::uint32_t count=0;
    for(std::size_t index=1;index<small.lengths_.size();++index){if(index<start||count==SmallStop)small.lengths_[index]=0;else{count=bits.read(SmallLengthBits);small.lengths_[index]=std::uint8_t(count==SmallStop?0:count);}}
    small.build();int runBits=0;for(auto value=std::int64_t(lengths_.size())-SmallRunEscape;value>0;value>>=1)++runBits;int last=0;
    for(std::size_t node=0;node<lengths_.size();){const auto value=small.decode(bits);if(value)lengths_[node++]=std::uint8_t(last=value-1);else{std::size_t run=bits.read(SmallLengthBits)+SmallRunBias;if(run==SmallRunEscape)run+=bits.read(runBits);run=std::min(run,lengths_.size()-node);std::fill_n(lengths_.begin()+node,run,std::uint8_t(last));node+=run;}}
    build();if(bits.overflow())throw MediaError(DiagnosticCode::ChdHuffmanTruncated);
}
void Huffman::build(){
    using namespace CodecConstants;std::array<std::uint32_t,HuffmanHistogramSize> histogram{};for(auto length:lengths_){if(length>maximumBits_)throw MediaError(DiagnosticCode::ChdHuffmanTree);++histogram[length];}std::uint32_t current=0;
    for(int length=WordBits;length>0;--length){const auto next=(current+histogram[std::size_t(length)])>>1;if(length!=1&&next+next!=current+histogram[std::size_t(length)])throw MediaError(DiagnosticCode::ChdHuffmanTree);histogram[std::size_t(length)]=current;current=next;}
    std::fill(lookup_.begin(),lookup_.end(),0);for(std::size_t symbol=0;symbol<lengths_.size();++symbol){const auto length=lengths_[symbol];if(!length)continue;const auto code=histogram[length]++;const auto shift=maximumBits_-length;const auto first=std::uint64_t(code)<<shift,end=std::uint64_t(code+1)<<shift;if(end>lookup_.size()||first>=end)throw MediaError(DiagnosticCode::ChdHuffmanTree);std::fill(lookup_.begin()+std::size_t(first),lookup_.begin()+std::size_t(end),(std::uint32_t(symbol)<<HuffmanEntryShift)|length);}
}
void Huffman::decompress(std::span<const std::uint8_t> source,std::span<std::uint8_t> destination,std::stop_token token){checkCancellation(token);Bitstream bits(source);importHuffman(bits);for(std::size_t index=0;index<destination.size();++index){if(!(index&CodecConstants::PollMask))checkCancellation(token);destination[index]=std::uint8_t(decode(bits));}bits.flush();if(bits.overflow())throw MediaError(DiagnosticCode::ChdHuffmanTruncated);}
}
