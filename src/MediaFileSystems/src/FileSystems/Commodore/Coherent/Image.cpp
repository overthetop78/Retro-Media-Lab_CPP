#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Coherent::Internal {
std::shared_ptr<const SectorBlock> Image::block(std::uint64_t number){
 checkCancellation(token);if(number>=blockCount())return {};
 if(const auto found=cache_.find(number);found!=cache_.end())return found->second;
 auto value=source_.block(number,token);if(value&&value->data.size()!=BlockSize)value.reset();cache_.emplace(number,value);return value;
}
bool Image::present(std::uint64_t offset,std::uint64_t length){
 if(offset/BlockSize>blockCount()||length>std::numeric_limits<std::uint64_t>::max()-offset)return false;
 if(!length)return offset/BlockSize<=blockCount();
 const auto last=(offset+length-1)/BlockSize;if(last>=blockCount())return false;
 for(auto number=offset/BlockSize;number<=last;++number)if(!block(number))return false;return true;
}
std::vector<std::uint8_t> Image::bytes(std::uint64_t offset,std::size_t length){
 std::vector<std::uint8_t> result(length);std::size_t destination=0;
 while(destination<length){checkCancellation(token);const auto absolute=offset+destination,within=absolute%BlockSize,count=std::min(std::uint64_t(length-destination),std::uint64_t(BlockSize)-within);if(const auto source=block(absolute/BlockSize))std::copy_n(source->data.begin()+within,std::size_t(count),result.begin()+destination);destination+=std::size_t(count);}return result;
}
}
