#include "Internal.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <QString>
#include <map>
#include <chrono>
namespace RetroMediaLab::MediaFileSystems::AmigaDos {
namespace {
using namespace Constants;
void put(MutableBytes data,std::size_t offset,std::uint32_t value){Binary::writeBig(data,offset,value);}
void text(MutableBytes data,std::size_t offset,std::size_t maximum,std::string_view value){
    const auto bytes=QString::fromUtf8(value.data(),qsizetype(value.size())).toLatin1();const auto length=std::min(maximum,std::size_t(bytes.size()));data[offset]=std::uint8_t(length);std::copy_n(bytes.begin(),length,data.begin()+offset+1);
}
Timestamp now(){
    using namespace std::chrono;const auto instant=system_clock::now();const auto date=year_month_day(floor<days>(instant));const hh_mm_ss clock(instant-floor<days>(instant));
    return {int(date.year()),int(unsigned(date.month())),int(unsigned(date.day())),int(clock.hours().count()),int(clock.minutes().count()),int(clock.seconds().count()),int(duration_cast<nanoseconds>(clock.subseconds()).count()),0};
}
class Builder {
    const MigrationPlan& plan_;Variant variant_;SectorGeometry geometry_;Timestamp default_;std::stop_token token_;std::uint32_t count_,root_,bitmap_,next_=BootBlockCount;
    std::map<std::uint32_t,std::vector<std::uint8_t>> blocks_;std::set<std::uint32_t> allocated_,checksums_;
    std::uint32_t allocate(){while(next_<count_){checkCancellation(token_);const auto number=next_++;if(allocated_.insert(number).second)return number;}throw MediaError(DiagnosticCode::AmigaDiskFull);}
    std::vector<std::uint8_t> metadata(int type,std::uint32_t number,std::uint32_t parent,std::string_view name,std::string_view comment,std::uint32_t attributes,std::optional<Timestamp> date={}){
        std::vector<std::uint8_t> data(BlockSize);put(data,PrimaryTypeOffset,HeaderPrimaryType);put(data,HeaderKeyOffset,number);put(data,ProtectionOffset,attributes);
        text(data,OrdinaryNameOffset,OrdinaryNameMaximumLength,name);text(data,LongNameOffset,CommentMaximumLength,comment);
        Internal::writeDate(data,DateOffset,date.value_or(default_));if(type==RootSecondaryType)Internal::writeDate(data,VolumeModifiedDateOffset,date.value_or(default_));
        put(data,ParentBlockOffset,parent);put(data,SecondaryTypeOffset,std::uint32_t(type));return data;
    }
    void insert(std::vector<std::uint8_t>& parent,std::string_view name,std::uint32_t number){
        const auto offset=DataPointersOffset+Internal::nameHash(name)*WordSize;auto current=Internal::unsignedInteger(parent,offset);
        if(current==0){put(parent,offset,number);return;}
        while(Internal::unsignedInteger(blocks_.at(current),HashChainOffset)!=0){checkCancellation(token_);current=Internal::unsignedInteger(blocks_.at(current),HashChainOffset);}put(blocks_.at(current),HashChainOffset,number);
    }
    void file(std::uint32_t header,const MigrationEntry& entry){
        if(!entry.content)throw MediaError(DiagnosticCode::AmigaInvalidEntry,{entry.sourcePath});
        const auto& content=*entry.content;if(content.size()>std::numeric_limits<std::uint32_t>::max())throw MediaError(DiagnosticCode::AmigaFileSize,{std::to_string(content.size())});
        put(blocks_.at(header),FileSizeOffset,std::uint32_t(content.size()));const auto payload=std::size_t(isFast(variant_)?BlockSize:OfsDataMaximumLength);std::vector<std::uint32_t> dataBlocks;
        for(std::size_t offset=0;offset<content.size();offset+=payload){
            checkCancellation(token_);const auto number=allocate();const auto length=std::min(payload,content.size()-offset);std::vector<std::uint8_t> data(BlockSize);
            const auto start=isFast(variant_)?0:OfsDataHeaderLength;
            if(!isFast(variant_)){put(data,PrimaryTypeOffset,OfsDataPrimaryType);put(data,HeaderKeyOffset,header);put(data,HighSequenceOffset,std::uint32_t(dataBlocks.size()+1));put(data,HashTableSizeOffset,std::uint32_t(length));checksums_.insert(number);}
            std::copy_n(content.begin()+offset,length,data.begin()+start);blocks_.emplace(number,std::move(data));dataBlocks.push_back(number);
        }
        if(!isFast(variant_))for(std::size_t i=0;i<dataBlocks.size();++i)put(blocks_.at(dataBlocks[i]),FirstReservedOffset,i+1<dataBlocks.size()?dataBlocks[i+1]:0);
        auto current=header;
        for(std::size_t start=0;start<dataBlocks.size();start+=RootHashTableEntryCount){
            const auto count=std::min(std::size_t(RootHashTableEntryCount),dataBlocks.size()-start);auto& data=blocks_.at(current);put(data,HighSequenceOffset,std::uint32_t(count));put(data,FirstReservedOffset,dataBlocks[start]);
            for(std::size_t i=0;i<count;++i)put(data,DataPointersOffset+(RootHashTableEntryCount-1-i)*WordSize,dataBlocks[start+i]);
            if(start+count>=dataBlocks.size())break;
            const auto extension=allocate();put(data,ExtensionBlockOffset,extension);std::vector<std::uint8_t> bytes(BlockSize);
            put(bytes,PrimaryTypeOffset,FileExtensionPrimaryType);put(bytes,HeaderKeyOffset,extension);put(bytes,ParentBlockOffset,header);put(bytes,SecondaryTypeOffset,std::uint32_t(FileSecondaryType));
            blocks_.emplace(extension,std::move(bytes));checksums_.insert(extension);current=extension;
        }
    }
    void entries(std::uint32_t parent,const std::vector<MigrationEntry>& entries,std::uint32_t depth){
        if(depth>MaximumDirectoryDepth)throw MediaError(DiagnosticCode::AmigaDepth,{std::to_string(depth)});
        for(const auto& entry:entries){
            checkCancellation(token_);if((entry.kind!=EntryKind::Directory&&entry.kind!=EntryKind::File)||!validName(entry.targetName))throw MediaError(DiagnosticCode::AmigaInvalidEntry,{entry.sourcePath});
            const auto number=allocate();blocks_.emplace(number,metadata(entry.kind==EntryKind::Directory?DirectorySecondaryType:FileSecondaryType,number,parent,entry.targetName,entry.comment,entry.rawAttributes,entry.modified));checksums_.insert(number);
            insert(blocks_.at(parent),entry.targetName,number);
            if(entry.kind==EntryKind::Directory)this->entries(number,entry.children,depth+1);else file(number,entry);
        }
    }
public:
    Builder(const MigrationPlan& plan,Variant variant,SectorGeometry geometry,Timestamp time,std::uint32_t count,std::stop_token token):plan_(plan),variant_(variant),geometry_(std::move(geometry)),default_(time),token_(token),count_(count),root_(count/BootBlockCount),bitmap_(root_+1){allocated_={0,1,root_,bitmap_};}
    SectorWritePlan build(){
        std::vector<std::uint8_t> boot(BlockSize*BootBlockCount);std::copy(DosSignature.begin(),DosSignature.end(),boot.begin());boot[DosVariantOffset]=std::uint8_t(variant_);put(boot,BootRootPointerOffset,root_);
        std::uint32_t sum=0;for(std::size_t offset=0;offset<boot.size();offset+=WordSize){if(offset==WordSize)continue;const auto previous=sum;sum+=Internal::unsignedInteger(boot,offset);if(sum<previous)++sum;}put(boot,WordSize,~sum);
        blocks_[0]={boot.begin(),boot.begin()+BlockSize};blocks_[1]={boot.begin()+BlockSize,boot.end()};
        blocks_[root_]=metadata(RootSecondaryType,0,0,plan_.volumeName,{},0);put(blocks_.at(root_),HashTableSizeOffset,RootHashTableEntryCount);put(blocks_.at(root_),BitmapValidityOffset,std::numeric_limits<std::uint32_t>::max());put(blocks_.at(root_),BitmapPointersOffset,bitmap_);checksums_.insert(root_);
        entries(root_,plan_.entries,0);std::vector<std::uint8_t> bitmap(BlockSize);
        for(std::uint32_t block=BootBlockCount;block<count_;++block){checkCancellation(token_);if(allocated_.contains(block))continue;const auto bit=block-BootBlockCount;const auto offset=BitmapDataOffset+(bit/WordBits)*WordSize;put(bitmap,offset,Internal::unsignedInteger(bitmap,offset)|(std::uint32_t(1)<<(bit%WordBits)));}
        blocks_[bitmap_]=std::move(bitmap);for(const auto number:checksums_)Internal::setChecksum(blocks_.at(number),ChecksumOffset);Internal::setChecksum(blocks_.at(bitmap_),0);
        SectorWritePlan plan;plan.geometry=geometry_;plan.blocks.reserve(count_);
        for(std::uint32_t logical=0;logical<count_;++logical){checkCancellation(token_);const auto track=logical/geometry_.sectorsPerTrack;auto found=blocks_.find(logical);auto data=found==blocks_.end()?std::vector<std::uint8_t>(BlockSize):std::move(found->second);plan.blocks.push_back({logical,{track/geometry_.heads,track%geometry_.heads,logical%geometry_.sectorsPerTrack},std::move(data)});}
        return plan;
    }
};
}
SectorWritePlan write(const MigrationPlan& plan,Variant variant,const SectorGeometry& geometry,std::optional<Timestamp> defaultTime,std::stop_token token){
    checkCancellation(token);if(variant!=Variant::Ofs&&variant!=Variant::Ffs)throw MediaError(DiagnosticCode::AmigaWriterVariant,{std::to_string(std::uint8_t(variant))});
    if(!validName(plan.volumeName))throw MediaError(DiagnosticCode::AmigaInvalidEntry,{std::string(RootPath)});
    const auto tracks=std::uint64_t(geometry.cylinders)*geometry.heads;
    if((geometry.formatId!=Formats::AmigaDos&&geometry.formatId!=Formats::AmigaDosHighDensity)||geometry.blockSize!=BlockSize||!tracks||!geometry.sectorsPerTrack||tracks>MaximumBitmapBlocks/geometry.sectorsPerTrack)throw MediaError(DiagnosticCode::AmigaWriterGeometry,{geometry.formatId});
    const auto count=std::uint32_t(tracks*geometry.sectorsPerTrack);if(count<MinimumWriterBlocks)throw MediaError(DiagnosticCode::AmigaWriterGeometry,{geometry.formatId});
    return Builder(plan,variant,geometry,defaultTime.value_or(now()),count,token).build();
}
}
