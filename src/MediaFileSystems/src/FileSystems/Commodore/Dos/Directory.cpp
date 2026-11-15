#include "Internal.h"
#include "../../../Primitives/PetsciiConstants.h"
#include <algorithm>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Internal {
namespace {
void readEntries(const Image& image,Bytes data,std::vector<FileSystemEntry>& entries,std::vector<Diagnostic>& warnings) {
    using namespace Constants;
    for(std::int32_t slot=0;slot<DirectorySlots;++slot) {
        checkCancellation(image.token);const auto offset=std::size_t(slot*DirectoryEntrySize);
        if(offset>data.size()||DirectoryEntrySize>data.size()-offset)continue;
        const auto raw=data[offset+TypeOffset],type=std::uint8_t(raw&BaseTypeMask);if(!type)continue;
        auto name=Petscii::decode(data.subspan(offset+NameOffset,NameLength));if(name.empty())continue;
        const auto declared=std::uint32_t(data[offset+DeclaredBlocksOffset])|std::uint32_t(data[offset+DeclaredBlocksOffset+1])<<(std::numeric_limits<std::uint8_t>::digits);
        auto content=file(image,data[offset+DataTrackOffset],data[offset+DataSectorOffset],warnings,name);
        FileSystemEntry entry;entry.name=std::move(name);entry.kind=EntryKind::File;entry.size=content.valid?content.content.size():declared*DataBytes;entry.rawAttributes=raw;
        if(type==Rel)entry.rawAttributes|=std::uint32_t(data[offset+RecordLengthOffset])<<RecordLengthShift;
        entry.storageReference=content.firstBlock.value_or(0);entry.metadataValid=content.valid;entry.content=std::move(content.content);
        const auto comment=std::size_t((raw&Closed)?0:1)+((raw&Locked)?LockedCommentIndex:0);
        entry.commentResourceKey=CommentKeys[comment];entry.commentTranslationDomain=TranslationDomain;entry.commentArguments={std::string(baseTypeName(raw))};
        entries.push_back(std::move(entry));
    }
}
}
bool plausible(const Image& image,std::int32_t track,std::int32_t number) {
    using namespace Constants;std::set<std::pair<std::int32_t,std::int32_t>> seen;std::size_t valid=0,invalid=0;
    while(track&&seen.size()<MaximumPlausibleChain&&seen.emplace(track,number).second) {
        checkCancellation(image.token);const auto read=sector(image,track,number);if(read.status!=SectorStatus::Success)return false;const auto data=Bytes(read.block->data);
        for(std::int32_t slot=0;slot<DirectorySlots;++slot) {
            const auto offset=std::size_t(slot*DirectoryEntrySize);if(offset>data.size()||DirectoryEntrySize>data.size()-offset)return false;
            const auto type=std::uint8_t(data[offset+TypeOffset]&BaseTypeMask);if(!type)continue;
            const auto name=Petscii::decode(data.subspan(offset+NameOffset,NameLength));const auto firstTrack=data[offset+DataTrackOffset],firstSector=data[offset+DataSectorOffset];
            const auto ok=type>=Seq&&type<=Cbm&&!name.empty()&&name.find(Petscii::Constants::Replacement)==std::string::npos&&(!firstTrack||logicalBlock(image.source,firstTrack,firstSector));
            if(ok)++valid;else ++invalid;
        }
        track=data[0];number=data[1];
    }
    return !invalid&&(valid||seen.size()==1);
}
std::vector<FileSystemEntry> directory(const Image& image,std::int32_t track,std::int32_t number,std::vector<Diagnostic>& warnings) {
    checkCancellation(image.token);std::vector<FileSystemEntry> entries;std::set<std::pair<std::int32_t,std::int32_t>> seen;
    while(track) {
        checkCancellation(image.token);const auto context=std::vector<std::string>{std::to_string(track),std::to_string(number)};
        if(!seen.emplace(track,number).second){warnings.push_back({DiagnosticCode::CbmDirectoryCycle,context});break;}
        const auto read=sector(image,track,number);
        if(read.status!=SectorStatus::Success) {
            if(read.status==SectorStatus::Truncated)warnings.push_back({DiagnosticCode::CbmDirectoryTruncated,{context[0],context[1],std::to_string(read.block->data.size())}});
            else warnings.push_back({read.status==SectorStatus::InvalidCoordinate?DiagnosticCode::CbmDirectoryCoordinate:DiagnosticCode::CbmDirectoryMissing,context});
            break;
        }
        readEntries(image,read.block->data,entries,warnings);track=read.block->data[0];number=read.block->data[1];
    }
    std::stable_sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.name<b.name;});return entries;
}
}
