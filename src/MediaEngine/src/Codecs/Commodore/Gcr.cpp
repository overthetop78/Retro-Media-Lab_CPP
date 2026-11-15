#include "Gcr.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreGcr {
using namespace Constants;
namespace {
std::uint64_t endOffset(std::int64_t offset,std::int64_t count,std::int64_t stride){
 if(offset<0||count<0||stride<0)throw MediaError(DiagnosticCode::GcrCoordinates,{std::to_string(offset),std::to_string(count),std::to_string(stride)});
 if(std::uint64_t(offset)>MaximumBits||std::uint64_t(count)>MaximumBits/ByteBits||std::uint64_t(stride)>MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
 const auto length=std::uint64_t(count)*ByteBits;if(stride&&length>(MaximumBits-std::uint64_t(offset))/std::uint64_t(stride))throw MediaError(DiagnosticCode::FluxSizeLimit);
 return std::uint64_t(offset)+length*std::uint64_t(stride);
}
}
std::optional<std::uint8_t> decodeNibble(Flux::BitView bits,std::int64_t offset,std::int64_t stride){
 endOffset(offset,0,stride);std::uint32_t code=0;
 for(std::int64_t bit=0;bit<NibbleBits;++bit){const auto position=std::uint64_t(offset)+std::uint64_t(bit)*std::uint64_t(stride);if(position>=bits.size())return {};code=(code<<1)|(bits[std::size_t(position)]?1:0);}
 const auto found=std::find(Encoding.begin(),Encoding.end(),code);if(found==Encoding.end())return {};return std::uint8_t(found-Encoding.begin());
}
std::optional<std::uint8_t> decodeByte(Flux::BitView bits,std::int64_t offset,std::int64_t stride){
 const auto high=decodeNibble(bits,offset,stride);if(!high)return {};const auto low=decodeNibble(bits,offset+NibbleBits*stride,stride);return low?std::optional<std::uint8_t>(std::uint8_t((*high<<NibbleShift)|*low)):std::nullopt;
}
DecodeResult decodeBytes(Flux::BitView bits,std::int64_t offset,std::int64_t count,std::int64_t stride,std::stop_token token){
 checkCancellation(token);const auto end=endOffset(offset,count,stride);if(end>bits.size())return {{},end};std::vector<std::uint8_t> result;result.reserve(std::size_t(count));
 for(std::int64_t index=0;index<count;++index){checkCancellation(token);const auto value=decodeByte(bits,offset+index*ByteBits*stride,stride);if(!value)return {{},end};result.push_back(*value);}return {std::move(result),end};
}
Flux::Bits encode(MediaFileSystems::Bytes bytes,std::stop_token token){
 checkCancellation(token);if(bytes.size()>MaximumBits/ByteBits)throw MediaError(DiagnosticCode::FluxSizeLimit);Flux::Bits result;result.reserve(bytes.size()*ByteBits);
 for(const auto value:bytes){checkCancellation(token);for(const auto nibble:{int(value>>NibbleShift),int(value&NibbleMask)})for(int bit=NibbleBits-1;bit>=0;--bit)result.push_back((Encoding[nibble]&(1<<bit))!=0);}return result;
}
void write(Flux::Bits& target,std::int64_t offset,MediaFileSystems::Bytes values,std::int64_t stride,std::stop_token token){
 checkCancellation(token);const auto end=endOffset(offset,std::int64_t(values.size()),stride);const auto encoded=encode(values,token);
 if(target.size()<end)target.resize(std::size_t(end));if(!encoded.empty()&&stride==0&&std::uint64_t(offset)>=target.size())throw MediaError(DiagnosticCode::GcrCoordinates,{std::to_string(offset),std::to_string(values.size()),std::to_string(stride)});
 for(std::size_t index=0;index<encoded.size();++index){checkCancellation(token);target[std::size_t(offset)+index*std::size_t(stride)]=encoded[index];}
}
std::uint8_t checksum(MediaFileSystems::Bytes values) noexcept {std::uint8_t result=0;for(const auto value:values)result^=value;return result;}
bool validChecksum(MediaFileSystems::Bytes values) noexcept {return checksum(values)==0;}
}
