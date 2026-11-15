#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AmigaDos::Internal {
FileData file(const SectorImage& image,Bytes header,std::uint32_t size,Variant variant,std::vector<Diagnostic>& warnings,std::stop_token token){
    FileData result{{},true};result.content.reserve(std::size_t(std::min(std::uint64_t(size),image.capacity())));
    std::vector<std::uint8_t> metadata(header.begin(),header.end());std::set<std::int32_t> extensions;
    while(true){
        checkCancellation(token);const auto sequence=std::clamp(integer(metadata,HighSequenceOffset),0,RootHashTableEntryCount);
        for(int i=0;i<sequence&&result.content.size()<size;++i){
            checkCancellation(token);const auto pointer=integer(metadata,DataPointersOffset+(RootHashTableEntryCount-1-i)*WordSize);
            const auto block=pointer>0&&std::uint64_t(pointer)<image.blockCount()?image.block(std::uint64_t(pointer),token):nullptr;
            if(!block){warnings.push_back({DiagnosticCode::AmigaMissingData,{std::to_string(pointer)}});result.valid=false;continue;}
            const Bytes bytes=block->data;
            if(!recordedIntegrity(*block)){warnings.push_back({DiagnosticCode::AmigaRecordedIntegrity,{std::to_string(pointer)}});result.valid=false;}
            if(isFast(variant)){const auto length=std::min(bytes.size(),std::size_t(size)-result.content.size());result.content.insert(result.content.end(),bytes.begin(),bytes.begin()+length);continue;}
            const auto type=integer(bytes,PrimaryTypeOffset);
            if(type!=OfsDataPrimaryType){warnings.push_back({DiagnosticCode::AmigaOfsType,{std::to_string(pointer),std::to_string(type)}});result.valid=false;}
            if(!validChecksum(bytes)){warnings.push_back({DiagnosticCode::AmigaDataChecksum,{std::to_string(pointer)}});result.valid=false;continue;}
            const auto declared=std::clamp(integer(bytes,HashTableSizeOffset),0,OfsDataMaximumLength);
            const auto length=std::min(std::size_t(declared),std::size_t(size)-result.content.size());result.content.insert(result.content.end(),bytes.begin()+OfsDataHeaderLength,bytes.begin()+OfsDataHeaderLength+length);
        }
        const auto extension=integer(metadata,ExtensionBlockOffset);if(extension==0)break;
        const auto block=extension>0&&std::uint64_t(extension)<image.blockCount()&&extensions.insert(extension).second?image.block(std::uint64_t(extension),token):nullptr;
        if(!block){warnings.push_back({DiagnosticCode::AmigaInvalidExtension,{std::to_string(extension)}});result.valid=false;break;}
        metadata=block->data;
        if(metadata.size()!=BlockSize||integer(metadata,PrimaryTypeOffset)!=FileExtensionPrimaryType||integer(metadata,SecondaryTypeOffset)!=FileSecondaryType||!validChecksum(metadata)){
            warnings.push_back({DiagnosticCode::AmigaExtensionChecksum,{std::to_string(extension)}});result.valid=false;break;
        }
        if(!recordedIntegrity(*block)){warnings.push_back({DiagnosticCode::AmigaRecordedIntegrity,{std::to_string(extension)}});result.valid=false;}
    }
    if(result.content.size()<size){warnings.push_back({DiagnosticCode::AmigaTruncatedFile,{std::to_string(size),std::to_string(result.content.size())}});result.valid=false;}
    return result;
}
}
