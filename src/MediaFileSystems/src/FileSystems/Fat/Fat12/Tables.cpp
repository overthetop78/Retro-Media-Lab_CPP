#include "Fat12Internal.h"
#include "../../../Primitives/Binary.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Fat12 {
using namespace Fat12Constants;
using namespace FatConstants;
std::optional<std::uint16_t> readTableEntry(Bytes fat,std::uint32_t cluster) {
    const auto offset=std::uint64_t(cluster)+cluster/Values::PackedPairDenominator;
    if(offset>=fat.size() || sizeof(std::uint16_t)>fat.size()-offset)return std::nullopt;
    const auto pair=Binary::little<std::uint16_t>(fat,std::size_t(offset));
    return std::uint16_t((cluster&1)==0?pair&Fat12Table::LastEndOfChain:pair>>Values::NibbleShift);
}
bool plausibleTableHeader(Bytes fat) {return fat.size()>=Values::FatHeaderLength && fat[0]>=Fat12Table::MinimumMediaDescriptor && fat[1]==Fat12Table::ReservedEntryByte && fat[Values::LastHeaderByte]==Fat12Table::ReservedEntryByte;}
namespace Internal {
std::optional<std::uint16_t> clusterEntry(Bytes fat, std::uint32_t cluster, TableKind kind) {
    if (kind == TableKind::Fat12) return readTableEntry(fat, cluster);
    if (cluster >= fat.size() / sizeof(std::uint16_t)) return {};
    return Binary::little<std::uint16_t>(fat, std::size_t(cluster) * sizeof(std::uint16_t));
}

Range sectors(const SectorImage& image,std::uint64_t first,std::uint32_t count,std::vector<Diagnostic>& warnings,std::stop_token token) {
    const auto blockSize=image.geometry().blockSize;
    if(std::uint64_t(count)*blockSize>Values::MaximumDecodedBytes)throw MediaError(DiagnosticCode::FatResourceLimit,{std::string(Values::SectorRangeName)});
    Range range;range.bytes.resize(std::size_t(count)*blockSize);range.present.resize(count,false);
    for(std::uint32_t i=0;i<count;++i) {
        checkCancellation(token);
        if(i>std::numeric_limits<std::uint64_t>::max()-first)throw MediaError(DiagnosticCode::ReadOutOfBounds);
        const auto logical=first+i;const auto block=image.block(logical,token);
        if(!block || block->data.size()!=blockSize) {
            warnings.push_back({DiagnosticCode::FatMissingSector,{std::to_string(logical),std::to_string(block?block->data.size():0),std::to_string(blockSize)}});continue;
        }
        std::copy(block->data.begin(),block->data.end(),range.bytes.begin()+std::size_t(i)*blockSize);range.present[i]=true;
        if(block->integrityValid==false || block->headerIntegrityValid==false || block->dataIntegrityValid==false) {
            range.trusted=false;warnings.push_back({DiagnosticCode::FatIntegrityInvalid,{std::to_string(logical)}});
        }
    }
    return range;
}
namespace {
std::uint32_t score(Bytes fat,std::uint32_t count,TableKind kind) {
    const auto reserved = kind == TableKind::Fat16 ? Fat16FirstReserved : Fat12Table::FirstReservedCluster;
    const auto last = kind == TableKind::Fat16 ? std::numeric_limits<std::uint16_t>::max() : Fat12Table::LastEndOfChain;
    std::uint32_t result=0;
    for(auto cluster=Fat12Table::FirstDataCluster;cluster<Fat12Table::FirstDataCluster+count;++cluster) {
        const auto value=clusterEntry(fat,cluster,kind);if(!value || *value==Fat12Table::FreeCluster)continue;
        const bool data=*value>=Fat12Table::FirstDataCluster && *value<Fat12Table::FirstDataCluster+count;
        const bool terminal=*value>=reserved && *value<=last;
        if(data || terminal)++result;
    }
    return result;
}
}
bool usableTable(Bytes fat,std::uint8_t descriptor,std::uint32_t count,TableKind kind) {
    if (kind == TableKind::Fat16) {
        constexpr std::uint16_t ReservedPrefix = 0xff00, EndOfChainMask = 0x3fff;
        const auto media = clusterEntry(fat, 0, kind), reserved = clusterEntry(fat, 1, kind);
        return media == (ReservedPrefix | descriptor) && reserved && (*reserved & EndOfChainMask) == EndOfChainMask;
    }
    return plausibleTableHeader(fat) || (descriptor==FatBootSectorLayout::UnknownMediaDescriptor && fat.size()>=Values::FatHeaderLength && fat[0]==0 && fat[1]==0 && fat[Values::LastHeaderByte]==0 && score(fat,count,kind)>0);
}
Range bestTable(const SectorImage& image,const Layout& layout,std::uint8_t descriptor,std::vector<Diagnostic>& warnings,std::stop_token token,TableKind kind) {
    std::optional<Range> best;std::uint32_t bestScore=0;std::vector<Diagnostic> selectedWarnings;
    for(std::uint32_t copy=0;copy<layout.fatCount;++copy) {
        std::vector<Diagnostic> candidateWarnings;
        auto candidate=sectors(image,layout.reservedSectors+copy*layout.sectorsPerFat,layout.sectorsPerFat,candidateWarnings,token);
        if(!candidate.complete() || !usableTable(candidate.bytes,descriptor,layout.clusterCount,kind))continue;
        const auto candidateScore=score(candidate.bytes,layout.clusterCount,kind);
        if(!best || (candidate.trusted && !best->trusted) || (candidate.trusted==best->trusted && candidateScore>bestScore)) {
            best=std::move(candidate);bestScore=candidateScore;selectedWarnings=std::move(candidateWarnings);
        }
    }
    if(!best)return sectors(image,layout.reservedSectors,layout.sectorsPerFat,warnings,token);
    warnings.insert(warnings.end(),selectedWarnings.begin(),selectedWarnings.end());return std::move(*best);
}
Chain clusterChain(ReadContext& context,const Range& fat,const Layout& layout,std::uint32_t first,std::string_view name) {
    Chain result;if(first<Fat12Table::FirstDataCluster)return result;
    result.valid=fat.valid();result.trusted=fat.trusted;auto cluster=first;
    const auto endOfChain = context.kind == TableKind::Fat16 ? Fat16FirstEndOfChain : Fat12Table::FirstEndOfChain;
    while(cluster<endOfChain) {
        checkCancellation(context.token);
        if(cluster<Fat12Table::FirstDataCluster || cluster>=layout.clusterCount+Fat12Table::FirstDataCluster) {
            context.warnings.push_back({DiagnosticCode::FatClusterOutsideRange,{std::string(name),std::to_string(cluster)}});result.valid=false;break;
        }
        if(!result.clusters.insert(cluster).second) {
            context.warnings.push_back({DiagnosticCode::FatCyclicChain,{std::string(name),std::to_string(cluster)}});result.valid=false;break;
        }
        const auto length=std::uint64_t(layout.sectorsPerCluster)*context.image.geometry().blockSize;
        if(length>context.remainingBytes) {
            context.warnings.push_back({DiagnosticCode::FatResourceLimit,{std::string(name)}});result.valid=false;break;
        }
        context.remainingBytes-=length;
        const auto start=layout.dataStart+(cluster-Fat12Table::FirstDataCluster)*layout.sectorsPerCluster;
        const auto range=sectors(context.image,start,layout.sectorsPerCluster,context.warnings,context.token);
        result.bytes.insert(result.bytes.end(),range.bytes.begin(),range.bytes.end());result.valid&=range.valid();result.trusted&=range.trusted;
        const auto next=clusterEntry(fat.bytes,cluster,context.kind);
        if(!next) {context.warnings.push_back({DiagnosticCode::FatTruncatedTable,{std::to_string(cluster)}});result.valid=false;break;}
        cluster=*next;
    }
    return result;
}
}
}
