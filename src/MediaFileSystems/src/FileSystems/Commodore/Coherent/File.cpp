#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Coherent::Internal {
namespace {
std::uint32_t capacity(std::uint32_t level){std::uint32_t result=1;while(level--)result*=PointersPerBlock;return result;}
void holes(std::vector<std::uint32_t>& result,std::uint32_t required,std::uint32_t count){result.insert(result.end(),std::min(count,required-std::uint32_t(result.size())),0);}
void indirect(Image& image,std::uint32_t block,std::uint32_t level,std::vector<std::uint32_t>& result,std::uint32_t required,std::set<std::uint32_t>& visited,std::vector<Diagnostic>& warnings,const std::string& name,bool& valid){
 checkCancellation(image.token);if(result.size()>=required)return;
 if(!block){holes(result,required,capacity(level));return;}
 const auto source=image.block(block);
 if(!source){warnings.push_back({DiagnosticCode::CoherentIndirectUnavailable,{name,std::to_string(block),std::to_string(level)}});holes(result,required,capacity(level));valid=false;return;}
 if(!visited.insert(block).second){warnings.push_back({DiagnosticCode::CoherentIndirectRepeated,{name,std::to_string(block),std::to_string(level)}});holes(result,required,capacity(level));valid=false;return;}
 for(std::uint32_t index=0;index<PointersPerBlock&&result.size()<required;++index){
  checkCancellation(image.token);const auto child=canonicalUInt32(Bytes(source->data).subspan(index*CanonicalLength,CanonicalLength));
  if(child>MaximumFileSize){warnings.push_back({DiagnosticCode::CoherentIndirectUnavailable,{name,std::to_string(child),std::to_string(level)}});holes(result,required,capacity(level-1));valid=false;continue;}
  if(level==1)result.push_back(child);else indirect(image,child,level-1,result,required,visited,warnings,name,valid);
 }
}
}
FileData file(Image& image,const Inode& inode,std::vector<Diagnostic>& warnings,const std::string& name){
 checkCancellation(image.token);if(inode.size>MaximumFileSize)throw MediaError(DiagnosticCode::CoherentFileTooLarge,{std::to_string(inode.size)});if(!inode.size)return {};
 const auto required=std::uint32_t((std::uint64_t(inode.size)+BlockSize-1)/BlockSize);std::vector<std::uint32_t> blocks;blocks.reserve(required);bool valid=true;
 for(std::uint32_t i=0;i<DirectCount&&blocks.size()<required;++i)blocks.push_back(inode.blocks[i]);
 std::set<std::uint32_t> visited;indirect(image,inode.blocks[SingleIndex],1,blocks,required,visited,warnings,name,valid);indirect(image,inode.blocks[DoubleIndex],DoubleLevel,blocks,required,visited,warnings,name,valid);indirect(image,inode.blocks[TripleIndex],TripleLevel,blocks,required,visited,warnings,name,valid);
 FileData result;result.valid=valid;result.content.resize(inode.size);std::size_t destination=0,missing=0;
 for(const auto number:blocks){
  checkCancellation(image.token);if(destination>=result.content.size())break;const auto count=std::min(std::size_t(BlockSize),result.content.size()-destination);
  if(number){if(const auto source=image.block(number))std::copy_n(source->data.begin(),count,result.content.begin()+destination);else{warnings.push_back({DiagnosticCode::CoherentDirectUnavailable,{name,std::to_string(number)}});missing+=count;result.valid=false;}}
  destination+=count;
 }
 if(destination<result.content.size()){missing+=result.content.size()-destination;result.valid=false;}
 if(missing)warnings.push_back({DiagnosticCode::CoherentMissingBytes,{name,std::to_string(missing)}});return result;
}
}
