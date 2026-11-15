#include "Bits.h"
#include "BitPacker.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Encoding {
using namespace Constants;
namespace {
void reserve(Flux::Bits& bits,std::uint64_t count){if(bits.size()>MaximumBits||count>MaximumBits-bits.size())throw MediaError(DiagnosticCode::FluxSizeLimit);bits.reserve(bits.size()+std::size_t(count));}
void reserveBytes(Flux::Bits& bits,std::size_t count,std::uint64_t factor){if(count>MaximumBits/BitsPerByte/factor)throw MediaError(DiagnosticCode::FluxSizeLimit);reserve(bits,std::uint64_t(count)*BitsPerByte*factor);}
int hex(char c){if(c>=Zero&&c<=HexDigitLast)return c-Zero;if(c>=HexUpperFirst&&c<=HexUpperLast)return c-HexUpperFirst+HexLetterOffset;if(c>=HexLowerFirst&&c<=HexLowerLast)return c-HexLowerFirst+HexLetterOffset;throw MediaError(DiagnosticCode::TrackHex);}
}
void raw(Flux::Bits& bits,MediaFileSystems::Bytes bytes){reserveBytes(bits,bytes.size(),1);for(const auto value:bytes)for(int i=BitsPerByte-1;i>=0;--i)bits.push_back((value&(1<<i))!=0);}
void rawHex(Flux::Bits& bits,std::string_view text){if(text.size()%DoubleCells)throw MediaError(DiagnosticCode::TrackHex);std::vector<std::uint8_t> bytes;bytes.reserve(text.size()/DoubleCells);for(std::size_t i=0;i<text.size();i+=DoubleCells)bytes.push_back(std::uint8_t(hex(text[i])*HexBase+hex(text[i+1])));raw(bits,bytes);}
void rawBits(Flux::Bits& bits,std::string_view text){reserve(bits,text.size());for(std::size_t i=0;i<text.size();++i){if(text[i]!=Zero&&text[i]!=One)throw MediaError(DiagnosticCode::TrackBinaryCharacter,{std::string(1,text[i]),std::to_string(i)});bits.push_back(text[i]==One);}}
void mfm(Flux::Bits& bits,MediaFileSystems::Bytes bytes,bool previous){reserveBytes(bits,bytes.size(),DoubleCells);for(const auto value:bytes)for(int i=BitsPerByte-1;i>=0;--i){const bool data=(value&(1<<i))!=0;bits.push_back(!previous&&!data);bits.push_back(data);previous=data;}}
void fm(Flux::Bits& bits,MediaFileSystems::Bytes bytes){reserveBytes(bits,bytes.size(),DoubleCells);for(const auto value:bytes)for(int i=BitsPerByte-1;i>=0;--i){bits.push_back(1);bits.push_back((value&(1<<i))!=0);}}
void doubleFm(Flux::Bits& bits,MediaFileSystems::Bytes bytes){reserveBytes(bits,bytes.size(),QuadCells);for(const auto value:bytes)for(int i=BitsPerByte-1;i>=0;--i){bits.push_back(0);bits.push_back(1);bits.push_back(0);bits.push_back((value&(1<<i))!=0);}}
void gap(Flux::Bits& bits,std::int64_t count,bool ones){if(count<0)throw MediaError(DiagnosticCode::TrackNegativeGap,{std::to_string(count)});reserve(bits,std::uint64_t(count));for(std::int64_t i=0;i<count;++i)bits.push_back(ones||(i&1)==0);}
std::vector<std::uint8_t> compactMfm(MediaFileSystems::Bytes data){Flux::Bits bits;mfm(bits,data);return packBits(bits);}
std::vector<std::uint8_t> compactFm(MediaFileSystems::Bytes data){Flux::Bits bits;fm(bits,data);return packBits(bits);}
}
