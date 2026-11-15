#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::ProDos::Internal {
using namespace Constants;
namespace {
bool index(const SectorImage& image,std::int32_t number,std::uint32_t count,std::string_view name,Storage storage,std::vector<std::int32_t>& pointers,std::set<std::int32_t>& indices,std::vector<Diagnostic>& warnings,std::stop_token token){
    const bool cyclic=!indices.insert(number).second;
    const auto block=!cyclic&&number>0&&std::uint64_t(number)<image.blockCount()?image.block(std::uint64_t(number),token):nullptr;
    if(!block||block->data.size()!=BlockSize){warnings.push_back({DiagnosticCode::ProDosIndex,{std::string(name),storageName(storage),std::to_string(number),std::string(cyclic?True:False)}});pointers.insert(pointers.end(),count,0);return false;}
    for(std::uint32_t pointer=0;pointer<std::min(count,IndexPointerCount);++pointer)pointers.push_back(readIndex(block->data,pointer));return true;
}
}
Content content(const SectorImage& image,Storage storage,std::int32_t key,std::uint32_t length,std::string_view name,std::vector<Diagnostic>& warnings,std::stop_token token){
    Content result;const auto required=(length+BlockSize-1)/BlockSize;std::vector<std::int32_t> pointers;pointers.reserve(required);
    if(storage==Storage::Seedling)pointers.push_back(key);
    else if(storage==Storage::Sapling)result.valid&=index(image,key,required,name,storage,pointers,result.indexBlocks,warnings,token);
    else if(storage==Storage::Tree){
        const bool seen=!result.indexBlocks.insert(key).second;
        const auto master=!seen&&key>0&&std::uint64_t(key)<image.blockCount()?image.block(std::uint64_t(key),token):nullptr;
        if(!master||master->data.size()!=BlockSize){warnings.push_back({DiagnosticCode::ProDosMaster,{std::string(name),storageName(storage),std::to_string(key)}});pointers.insert(pointers.end(),required,0);result.valid=false;}
        else for(std::uint32_t child=0;child<IndexPointerCount&&pointers.size()<required;++child){
            checkCancellation(token);const auto count=std::min(IndexPointerCount,required-std::uint32_t(pointers.size()));const auto number=readIndex(master->data,child);
            if(!number)pointers.insert(pointers.end(),count,0);else result.valid&=index(image,number,count,name,storage,pointers,result.indexBlocks,warnings,token);
        }
    }
    for(std::size_t logical=0;logical<std::min<std::size_t>(required,pointers.size());++logical){
        checkCancellation(token);const auto number=pointers[logical];
        if(!number){result.data.insert(result.data.end(),BlockSize,0);continue;}
        const auto block=number>=0&&std::uint64_t(number)<image.blockCount()?image.block(std::uint64_t(number),token):nullptr;
        if(!block||block->data.size()!=BlockSize){warnings.push_back({DiagnosticCode::ProDosData,{std::string(name),storageName(storage),std::to_string(number)}});result.data.insert(result.data.end(),BlockSize,0);result.valid=false;}
        else{result.dataBlocks.insert(number);result.data.insert(result.data.end(),block->data.begin(),block->data.end());}
    }
    if(result.data.size()<length){warnings.push_back({DiagnosticCode::ProDosTruncated,{std::string(name),storageName(storage),std::to_string(result.data.size()),std::to_string(length)}});result.valid=false;}
    if(result.data.size()>length)result.data.resize(length);return result;
}
}
