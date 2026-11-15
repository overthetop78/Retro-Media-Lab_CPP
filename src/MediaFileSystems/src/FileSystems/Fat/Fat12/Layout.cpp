#include "Fat12Internal.h"
#include "../../../Primitives/Binary.h"
#include "../../../Primitives/MetadataConstants.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Fat12 {
using namespace FatConstants;
using namespace Fat12Constants;
namespace Internal {
bool same(std::string_view a,std::string_view b) {
    if(a.size()!=b.size())return false;
    const auto lower=[](char value){return value>=MetadataConstants::UpperFirst && value<=MetadataConstants::UpperLast?char(value+MetadataConstants::CaseDelta):value;};
    return std::equal(a.begin(),a.end(),b.begin(),[&](char left,char right){return lower(left)==lower(right);});
}
bool supported(std::string_view format) {return std::any_of(Fat12Formats::Supported.begin(),Fat12Formats::Supported.end(),[&](auto id){return same(format,id);});}
bool dosPlus(const SectorImage& image) {return same(image.geometry().formatId,Fat12Formats::AcornDosPlus800) || (same(image.geometry().formatId,Fat12Formats::Jfd) && image.geometry().blockSize==Values::DosPlusSectorSize);}
MediaError unsupported(const SectorImage& image,const std::shared_ptr<const SectorBlock>& boot) {
    using namespace FatBootSectorLayout;
    if(!boot || boot->data.size()<MinimumLength)return MediaError(DiagnosticCode::FatUnsupportedBoot,{image.geometry().formatId});
    const Bytes bytes=boot->data;
    return MediaError(DiagnosticCode::FatUnsupportedLayout,{image.geometry().formatId,std::to_string(Binary::little<std::uint16_t>(bytes,BytesPerSectorOffset)),std::to_string(bytes[SectorsPerClusterOffset]),
        std::to_string(Binary::little<std::uint16_t>(bytes,ReservedSectorCountOffset)),std::to_string(bytes[FatCountOffset]),std::to_string(Binary::little<std::uint16_t>(bytes,RootEntryCountOffset)),std::to_string(Binary::little<std::uint16_t>(bytes,SectorsPerFatOffset))});
}
}
namespace {
std::optional<Layout> legacy(Bytes boot,std::uint64_t available,std::string_view format) {
    if(boot.empty() || std::all_of(boot.begin(),boot.end(),[&](auto byte){return byte==boot.front();}))return std::nullopt;
    for(const auto& entry:Fat12Formats::Legacy)if(Internal::same(format,entry.formatId) && available>=entry.totalSectors) {
        const auto rootSectors=(entry.rootEntries*FatDirectoryLayout::EntrySize+FatBootSectorLayout::SectorSize-1)/FatBootSectorLayout::SectorSize;
        const auto rootStart=Values::LegacyReservedCount+Values::LegacyFatCopies*entry.sectorsPerFat,dataStart=rootStart+rootSectors;
        return Layout{Values::LegacyReservedCount,entry.sectorsPerFat,rootStart,rootSectors,dataStart,entry.sectorsPerCluster,(entry.totalSectors-dataStart)/entry.sectorsPerCluster,Values::HistoricalTableFatCount};
    }
    return std::nullopt;
}
}
std::optional<Layout> readLayout(Bytes boot,std::uint64_t available,std::string_view format) {
    using namespace FatBootSectorLayout;
    if(Internal::same(format,Fat12Formats::AcornDosPlus800) || (Internal::same(format,Fat12Formats::Jfd) && boot.size()==Values::DosPlusSectorSize)) {
        if(available!=Values::DosPlusSectorCount || boot.size()!=Values::DosPlusSectorSize || boot[0]!=Values::DosPlusDescriptor || boot[1]!=Fat12Table::ReservedEntryByte || boot[Values::PackedPairDenominator]!=Fat12Table::ReservedEntryByte)return std::nullopt;
        const auto dataStart=Values::DosPlusFatSectors+Values::DosPlusRootSectors;
        return Layout{0,Values::DosPlusFatSectors,Values::DosPlusFatSectors,Values::DosPlusRootSectors,dataStart,1,Values::DosPlusSectorCount-dataStart,1};
    }
    if(Internal::same(format,Fat12Formats::ApricotPcXi315))return legacy(boot,available,format);
    if(boot.size()<MinimumLength)return std::nullopt;
    const auto sectorSize=Binary::little<std::uint16_t>(boot,BytesPerSectorOffset);
    const auto perCluster=boot[SectorsPerClusterOffset];const auto reserved=Binary::little<std::uint16_t>(boot,ReservedSectorCountOffset);const auto copies=boot[FatCountOffset];
    const auto rootEntries=Binary::little<std::uint16_t>(boot,RootEntryCountOffset);std::uint32_t total=Binary::little<std::uint16_t>(boot,TotalSectors16Offset);
    if(total==0){total=Binary::little<std::uint32_t>(boot,TotalSectors32Offset);if(total>std::numeric_limits<std::uint16_t>::max())return std::nullopt;}
    const auto perFat=Binary::little<std::uint16_t>(boot,SectorsPerFatOffset);
    if(sectorSize!=SectorSize || perCluster==0 || reserved==0 || copies==0 || rootEntries==0 || total==0 || total>available || perFat==0)return legacy(boot,available,format);
    const auto rootSectors=(rootEntries*FatDirectoryLayout::EntrySize+SectorSize-1)/SectorSize;
    const std::uint32_t rootStart=reserved+copies*perFat;const auto dataStart=rootStart+rootSectors;
    if(dataStart>=total)return std::nullopt;
    const auto clusters=(total-dataStart)/perCluster;
    if(clusters==0 || clusters>=Fat12Layout::MaximumClusterCount)return std::nullopt;
    return Layout{reserved,perFat,rootStart,rootSectors,dataStart,perCluster,clusters,copies};
}
}
