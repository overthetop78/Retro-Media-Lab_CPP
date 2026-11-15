#include "Internal.h"
#include "../../../Primitives/Binary.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Adfs::Internal {
using namespace Constants;
bool readBytes(const SectorImage& image,const FileCore::AddressResolver& resolver,std::int32_t address,MutableBytes output,bool& integrity,std::vector<Diagnostic>* warnings,std::stop_token token){
    std::size_t copied=0;const auto size=image.geometry().blockSize;
    if(size==0)return false;
    while(copied<output.size()){
        checkCancellation(token);const auto position=resolver.resolve(address,std::int64_t(copied),token);if(!position)return false;
        const auto number=*position/size,intra=*position%size;
        const auto block=image.block(number,token);if(!block||block->data.size()!=size)return false;
        if(!validIntegrity(*block)){integrity=false;if(warnings)warnings->push_back({DiagnosticCode::AdfsIntegrity,{std::to_string(number)}});}
        const auto count=std::min(std::size_t(size-intra),output.size()-copied);
        std::copy_n(block->data.begin()+std::size_t(intra),count,output.begin()+copied);copied+=count;
    }
    return true;
}
std::optional<std::vector<std::uint8_t>> directoryBytes(const SectorImage& image,const FileCore::AddressResolver& resolver,std::int32_t address,bool& integrity,std::vector<Diagnostic>* warnings,std::stop_token token){
    const auto size=image.geometry().blockSize==UnitSize?BbcDirectorySize:DirectorySize;
    std::vector<std::uint8_t> data(size);
    if(!readBytes(image,resolver,address,data,integrity,warnings,token))return {};
    const std::string_view header(reinterpret_cast<const char*>(data.data()+1),SignatureSize);
    const std::string_view footer(reinterpret_cast<const char*>(data.data()+size-FooterOffset),SignatureSize);
    if((header!=Hugo&&header!=Nick)||header!=footer||data[0]!=data[size-SequenceOffset])return {};
    return data;
}
Directory directory(Context& context,std::int32_t address,std::set<std::int32_t>& visited,std::uint32_t depth,bool parentIntegrity,std::stop_token token){
    checkCancellation(token);
    if(depth>MaximumDepth){context.warnings.push_back({DiagnosticCode::AdfsDepth,{std::to_string(depth)}});return {};}
    if(!visited.insert(address).second){context.warnings.push_back({DiagnosticCode::AdfsCycle,{std::to_string(address)}});return {};}
    Directory result;result.integrity=parentIntegrity;
    const auto data=directoryBytes(context.image,*context.resolver,address,result.integrity,&context.warnings,token);
    if(!data)throw MediaError(DiagnosticCode::AdfsInvalidDirectory,{std::to_string(address)});
    const Bytes bytes=*data;const auto bbc=context.image.geometry().blockSize==UnitSize;
    for(int index=0;index<(bbc?BbcEntryCount:EntryCount);++index){
        checkCancellation(token);const auto offset=std::size_t(EntriesOffset+index*EntrySize);if(bytes[offset]==0)break;
        FileSystemEntry entry;entry.name=FileCore::decodeAdfsName(bytes.subspan(offset,NameLength));if(entry.name.empty())continue;
        const auto load=Binary::little<std::uint32_t>(bytes,offset+LoadOffset),execute=Binary::little<std::uint32_t>(bytes,offset+ExecuteOffset);
        const auto length=Binary::little<std::uint32_t>(bytes,offset+LengthOffset);
        std::int32_t indirect=0;for(int byte=0;byte<UInt24Size;++byte)indirect|=std::int32_t(bytes[offset+AddressOffset+byte])<<(byte*BitsPerByte);
        auto attributes=std::uint32_t(bytes[offset+AttributesOffset]);
        if(bbc){attributes=0;for(int bit=0;bit<SignatureSize;++bit)attributes|=((bytes[offset+bit]>>BbcAttributeShift)&1)<<bit;}
        entry.kind=(attributes&DirectoryAttribute)?EntryKind::Directory:EntryKind::File;
        entry.size=entry.kind==EntryKind::Directory?0:length;entry.modified=decodeTime(load,execute);entry.comment=describe(load,execute);
        entry.rawAttributes=attributes;entry.storageReference=indirect;
        const bool addressValid=context.resolver->resolve(indirect,0,token).has_value();entry.metadataValid=addressValid&&result.integrity;
        if(!result.integrity)entry.dataValid=false;
        if(entry.kind==EntryKind::Directory&&addressValid){
            try{auto child=directory(context,indirect,visited,depth+1,result.integrity,token);entry.children=std::move(child.entries);entry.metadataValid=entry.metadataValid&&child.integrity;}
            catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData)throw;auto diagnostic=error.diagnostic();diagnostic.context.push_back(entry.name);context.warnings.push_back(diagnostic);entry.diagnostics.push_back(std::move(diagnostic));entry.metadataValid=false;}
        }else if(entry.kind==EntryKind::File)entry.content=file(context,indirect,length,entry.name,entry,token);
        result.entries.push_back(std::move(entry));
    }
    result.title=FileCore::decodeAdfsName(bytes.subspan(bbc?BbcTitleOffset:TitleOffset,TitleLength));
    result.name=FileCore::decodeAdfsName(bytes.subspan(bbc?BbcNameOffset:DirectoryNameOffset,NameLength));
    std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto& a,const auto& b){
        if((a.kind==EntryKind::Directory)!=(b.kind==EntryKind::Directory))return a.kind==EntryKind::Directory;
        return std::lexicographical_compare(a.name.begin(),a.name.end(),b.name.begin(),b.name.end(),[](char x,char y){const auto fold=[](char value){return value>=LowerFirst&&value<=LowerLast?char(value-LowerFirst+UpperFirst):value;};return fold(x)<fold(y);});
    });return result;
}
}
