#include "Fat12Internal.h"
#include "../../../Primitives/Binary.h"
#include "../../../Primitives/MetadataConstants.h"
#include "../../../Primitives/Identifiers.h"
#include <set>
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Fat12 {
using namespace FatConstants;
using namespace Fat12Constants;
namespace {
using namespace FatBootSectorLayout;
using namespace FatDirectoryLayout;
using namespace Values;
std::uint64_t roundUp(std::uint64_t value,std::uint64_t divisor) { return value/divisor+(value%divisor!=0); }
struct WritableLayout { Layout layout;std::uint32_t rootEntries;std::uint8_t descriptor; };
WritableLayout writableLayout(std::uint32_t total,const std::string& format) {
    const std::uint32_t perCluster=total<=SmallWriterSectors?1:total<=MediumWriterSectors?DoubleSectorCluster:total<=LargeWriterSectors?1:DoubleSectorCluster;
    const auto roots=total<=SmallWriterSectors?SmallRootEntries:total<=MediumWriterSectors?MediumRootEntries:LargeRootEntries;
    const auto rootSectors=std::uint32_t(roundUp(std::uint64_t(roots)*EntrySize,SectorSize));
    std::uint32_t perFat=1;
    for (;;) {
        const auto rootStart=FirstFatLogicalBlock+WriterFatCopies*perFat;
        const auto dataStart=rootStart+rootSectors;
        if(dataStart>=total)throw MediaError(DiagnosticCode::FatUnsupportedGeometry,{format});
        const auto clusters=(total-dataStart)/perCluster;
        const auto required=std::uint32_t(roundUp(std::uint64_t(clusters+Fat12Table::FirstDataCluster)*PackedPairNumerator,PackedPairDenominator*SectorSize));
        if(required<=perFat) {
            if(!clusters || clusters>=Fat12Layout::MaximumClusterCount)throw MediaError(DiagnosticCode::FatUnsupportedGeometry,{format});
            return {{FirstFatLogicalBlock,perFat,rootStart,rootSectors,dataStart,perCluster,clusters,WriterFatCopies},roots,total>=LargeWriterSectors?HighDensityMedia:DoubleDensityMedia};
        }
        perFat=required;
    }
}
void padded(MutableBytes bytes,std::string_view value) {
    std::fill(bytes.begin(),bytes.end(),std::uint8_t(SpacePadding));
    std::copy_n(value.begin(),std::min(value.size(),bytes.size()),bytes.begin());
}
void shortName(MutableBytes bytes,std::string_view name) {
    std::fill_n(bytes.begin(),VolumeLabelLength,std::uint8_t(SpacePadding));
    const auto dot=name.find(ExtensionSeparator);
    const auto stem=name.substr(0,dot);std::copy(stem.begin(),stem.end(),bytes.begin());
    if(dot!=std::string_view::npos) {const auto extension=name.substr(dot+1);std::copy(extension.begin(),extension.end(),bytes.begin()+ExtensionOffset);}
}
void timestamp(MutableBytes bytes,const std::optional<Timestamp>& value) {
    if(!value)return;
    const auto& date=*value;
    const auto year=std::clamp(date.year,int(MinimumFatYear),int(MaximumFatYear));
    const auto packedDate=std::uint16_t((year-MinimumFatYear)<<FatDateTime::YearShift | date.month<<FatDateTime::MonthShift | date.day);
    const auto packedTime=std::uint16_t(date.hour<<FatDateTime::HourShift | date.minute<<FatDateTime::MinuteShift | date.second/FatDateTime::SecondMultiplier);
    Binary::writeLittle(bytes,ModifiedDateOffset,packedDate);Binary::writeLittle(bytes,ModifiedTimeOffset,packedTime);
}
struct Node {const MigrationEntry* entry;std::uint32_t parent,first,count;std::vector<Node> children;};
class Builder {
public:
    Builder(const MigrationPlan& plan,const TargetGeometry& target,std::uint32_t total,std::stop_token token)
        : plan_(plan),target_(target),total_(total),writable_(writableLayout(total,target.geometry.formatId)),token_(token) {
        blocks_.reserve(total);
        for(std::uint32_t logical=0;logical<total;++logical) {
            checkCancellation(token_);const auto track=logical/target.geometry.sectorsPerTrack;
            blocks_.push_back({logical,{track/target.geometry.heads,track%target.geometry.heads,logical%target.geometry.sectorsPerTrack+FirstSectorNumber},std::vector<std::uint8_t>(SectorSize)});
        }
    }
    SectorWritePlan build() {
        const auto nodes=createNodes(plan_.entries,0,0);
        if(nodes.size()+1>writable_.rootEntries)throw MediaError(DiagnosticCode::FatDiskFull);
        boot();
        const auto& layout=writable_.layout;
        std::vector<std::uint8_t> fat(std::size_t(layout.sectorsPerFat)*SectorSize);
        fat[0]=writable_.descriptor;fat[1]=Fat12Table::ReservedEntryByte;fat[LastHeaderByte]=Fat12Table::ReservedEntryByte;
        for(const auto& node:nodes)writeNode(node,fat);
        std::vector<std::uint8_t> root(std::size_t(layout.rootSectors)*SectorSize);
        padded(MutableBytes(root).first(VolumeLabelLength),plan_.volumeName);root[AttributesOffset]=VolumeLabelAttribute;
        for(std::size_t index=0;index<nodes.size();++index)directoryEntry(MutableBytes(root).subspan((index+1)*EntrySize,EntrySize),nodes[index]);
        for(std::uint32_t sector=0;sector<layout.rootSectors;++sector)copySector(layout.rootStart+sector,Bytes(root).subspan(std::size_t(sector)*SectorSize,SectorSize));
        for(std::uint32_t copy=0;copy<layout.fatCount;++copy)
            for(std::uint32_t sector=0;sector<layout.sectorsPerFat;++sector)copySector(layout.reservedSectors+copy*layout.sectorsPerFat+sector,Bytes(fat).subspan(std::size_t(sector)*SectorSize,SectorSize));
        return {target_.geometry,std::move(blocks_),std::uint64_t(total_)*SectorSize,total_};
    }
private:
    std::vector<Node> createNodes(const std::vector<MigrationEntry>& entries,std::uint32_t parent,std::uint32_t depth) {
        if(depth>Fat12DirectoryReader::MaximumDepth)throw MediaError(DiagnosticCode::FatInvalidEntry,{std::string(MetadataConstants::RootPath)});
        std::set<std::string> names;std::vector<Node> result;result.reserve(entries.size());
        for(const auto& entry:entries) {
            checkCancellation(token_);
            if(!validShortName(entry.targetName) || !names.insert(entry.targetName).second ||
               (entry.kind!=EntryKind::File && entry.kind!=EntryKind::Directory) ||
               (entry.kind==EntryKind::File && (!entry.content || !entry.children.empty())) ||
               (entry.modified && !validTimestamp(*entry.modified)))throw MediaError(DiagnosticCode::FatInvalidEntry,{entry.sourcePath});
            const auto clusterBytes=std::uint64_t(writable_.layout.sectorsPerCluster)*SectorSize;
            const auto size=entry.kind==EntryKind::Directory?(std::uint64_t(entry.children.size())+SpecialDirectoryEntries)*EntrySize:entry.content->size();
            auto count=roundUp(size,clusterBytes);if(entry.kind==EntryKind::Directory)count=std::max<std::uint64_t>(1,count);
            if(count>writable_.layout.clusterCount || nextCluster_+count>Fat12Table::FirstDataCluster+writable_.layout.clusterCount)throw MediaError(DiagnosticCode::FatDiskFull);
            Node node{&entry,parent,count?nextCluster_:0,std::uint32_t(count),{}};nextCluster_+=std::uint32_t(count);
            node.children=createNodes(entry.children,node.first,depth+1);result.push_back(std::move(node));
        }
        return result;
    }
    void boot() {
        auto& bytes=blocks_[BootLogicalBlock].data;MutableBytes data=bytes;
        bytes[0]=ShortJumpOpcode;bytes[1]=JumpDisplacement;bytes[BootJumpPaddingIndex]=JumpPadding;
        const auto oem=target_.isMsx?OemMsx:OemRetroMediaLab;std::copy(oem.begin(),oem.end(),bytes.begin()+OemOffset);
        Binary::writeLittle(data,BytesPerSectorOffset,std::uint16_t(SectorSize));bytes[SectorsPerClusterOffset]=std::uint8_t(writable_.layout.sectorsPerCluster);
        Binary::writeLittle(data,ReservedSectorCountOffset,std::uint16_t(FirstFatLogicalBlock));bytes[FatCountOffset]=std::uint8_t(WriterFatCopies);
        Binary::writeLittle(data,RootEntryCountOffset,std::uint16_t(writable_.rootEntries));Binary::writeLittle(data,TotalSectors16Offset,std::uint16_t(total_));
        bytes[MediaDescriptorOffset]=writable_.descriptor;Binary::writeLittle(data,SectorsPerFatOffset,std::uint16_t(writable_.layout.sectorsPerFat));
        Binary::writeLittle(data,SectorsPerTrackOffset,std::uint16_t(target_.geometry.sectorsPerTrack));Binary::writeLittle(data,HeadCountOffset,std::uint16_t(target_.geometry.heads));
        padded(data.subspan(VolumeLabelOffset,VolumeLabelLength),plan_.volumeName);bytes[BootSignatureOffset]=BootSignatureLow;bytes[BootSignatureOffset+1]=BootSignatureHigh;
    }
    void copySector(std::uint32_t logical,Bytes bytes) {checkCancellation(token_);std::copy(bytes.begin(),bytes.end(),blocks_[logical].data.begin());}
    void clusterData(std::uint32_t cluster,Bytes bytes) {
        const auto first=writable_.layout.dataStart+(cluster-Fat12Table::FirstDataCluster)*writable_.layout.sectorsPerCluster;
        for(std::uint32_t sector=0;sector<writable_.layout.sectorsPerCluster;++sector) {
            const auto offset=std::size_t(sector)*SectorSize;if(offset>=bytes.size())break;
            copySector(first+sector,bytes.subspan(offset,std::min<std::size_t>(SectorSize,bytes.size()-offset)));
        }
    }
    static void tableEntry(MutableBytes fat,std::uint32_t cluster,std::uint32_t value) {
        const auto offset=cluster+cluster/PackedPairDenominator;
        if(cluster%PackedPairDenominator==0) {fat[offset]=std::uint8_t(value);fat[offset+1]=std::uint8_t((fat[offset+1]&HighNibble)|((value>>BinaryConstants::BitsPerByte)&LowNibble));}
        else {fat[offset]=std::uint8_t((fat[offset]&LowNibble)|(value<<NibbleShift));fat[offset+1]=std::uint8_t(value>>NibbleShift);}
    }
    static void directoryEntry(MutableBytes bytes,const Node& node) {
        shortName(bytes,node.entry->targetName);bytes[AttributesOffset]=node.entry->kind==EntryKind::Directory?DirectoryAttribute:ArchiveAttribute;
        timestamp(bytes,node.entry->modified);Binary::writeLittle(bytes,FirstClusterOffset,std::uint16_t(node.first));
        if(node.entry->kind==EntryKind::File)Binary::writeLittle(bytes,FileSizeOffset,std::uint32_t(node.entry->content->size()));
    }
    static void specialEntry(MutableBytes bytes,std::string_view name,std::uint32_t cluster) {
        std::fill(bytes.begin(),bytes.end(),std::uint8_t(SpacePadding));std::copy(name.begin(),name.end(),bytes.begin());
        bytes[AttributesOffset]=DirectoryAttribute;Binary::writeLittle(bytes,FirstClusterOffset,std::uint16_t(cluster));
    }
    void writeNode(const Node& node,MutableBytes fat) {
        checkCancellation(token_);
        for(std::uint32_t index=0;index<node.count;++index)tableEntry(fat,node.first+index,index+1<node.count?node.first+index+1:Fat12Table::LastEndOfChain);
        const auto clusterBytes=std::size_t(writable_.layout.sectorsPerCluster)*SectorSize;
        if(node.entry->kind==EntryKind::File) {
            const Bytes content=*node.entry->content;
            for(std::uint32_t index=0;index<node.count;++index){const auto offset=std::size_t(index)*clusterBytes;clusterData(node.first+index,content.subspan(offset,std::min(clusterBytes,content.size()-offset)));}
            return;
        }
        std::vector<std::uint8_t> directory(std::size_t(node.count)*clusterBytes);MutableBytes data=directory;
        specialEntry(data.first(EntrySize),CurrentDirectoryName,node.first);specialEntry(data.subspan(EntrySize,EntrySize),ParentDirectoryName,node.parent);
        for(std::size_t index=0;index<node.children.size();++index)directoryEntry(data.subspan((index+SpecialDirectoryEntries)*EntrySize,EntrySize),node.children[index]);
        for(std::uint32_t index=0;index<node.count;++index)clusterData(node.first+index,Bytes(directory).subspan(std::size_t(index)*clusterBytes,clusterBytes));
        for(const auto& child:node.children)writeNode(child,fat);
    }
    const MigrationPlan& plan_;const TargetGeometry& target_;std::uint32_t total_;WritableLayout writable_;std::stop_token token_;
    std::uint32_t nextCluster_=Fat12Table::FirstDataCluster;std::vector<SectorWriteBlock> blocks_;
};
}
bool validShortName(std::string_view name) {
    if(name.empty() || name==FatDirectoryLayout::CurrentDirectoryName || name==FatDirectoryLayout::ParentDirectoryName)return false;
    const auto dot=name.find(FatDirectoryLayout::ExtensionSeparator);
    const auto stem=name.substr(0,dot);const auto extension=dot==std::string_view::npos?std::string_view{}:name.substr(dot+1);
    if(stem.empty() || stem.size()>FatDirectoryLayout::NameLength || extension.size()>FatDirectoryLayout::ExtensionLength)return false;
    return std::all_of(name.begin(),name.end(),[](char value){return value==FatDirectoryLayout::ExtensionSeparator || (value>=MetadataConstants::UpperFirst && value<=MetadataConstants::UpperLast) || (value>=MetadataConstants::DigitFirst && value<=MetadataConstants::DigitLast) || Values::AllowedPunctuation.find(value)!=std::string_view::npos;}) && extension.find(FatDirectoryLayout::ExtensionSeparator)==std::string_view::npos;
}
bool validVolumeName(std::string_view name) {
    return name.size()<=FatBootSectorLayout::VolumeLabelLength && std::all_of(name.begin(),name.end(),[](char value){return value==FatBootSectorLayout::SpacePadding || (value>=MetadataConstants::UpperFirst && value<=MetadataConstants::UpperLast) || (value>=MetadataConstants::DigitFirst && value<=MetadataConstants::DigitLast);});
}
SectorWritePlan write(const MigrationPlan& plan,const TargetGeometry& target,std::stop_token token) {
    checkCancellation(token);const auto& geometry=target.geometry;
    if(Identifiers::isBlank(geometry.formatId))throw MediaError(DiagnosticCode::SectorInvalidFormatId,{geometry.formatId});
    if(geometry.blockSize!=FatBootSectorLayout::SectorSize || !geometry.cylinders || !geometry.heads || !geometry.sectorsPerTrack ||
       geometry.cylinders>Values::MaximumWriterSectors || geometry.heads>Values::MaximumWriterSectors || geometry.sectorsPerTrack>Values::MaximumWriterSectors)
        throw MediaError(DiagnosticCode::FatUnsupportedGeometry,{geometry.formatId});
    const auto total=std::uint64_t(geometry.cylinders)*geometry.heads*geometry.sectorsPerTrack;
    if(total>Values::MaximumWriterSectors)throw MediaError(DiagnosticCode::FatUnsupportedGeometry,{geometry.formatId});
    if(!validVolumeName(plan.volumeName))throw MediaError(DiagnosticCode::FatInvalidEntry,{std::string(MetadataConstants::RootPath)});
    return Builder(plan,target,std::uint32_t(total),token).build();
}
}
