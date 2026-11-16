#include "Container.h"
#include "Layout.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Qcow2 {
namespace {
std::uint32_t selectVersion(const MediaDocument& document){
    using namespace Constants;if(sameIdentifier(document.formatId(),FormatId)){const auto found=document.metadata().find(std::string(VersionKey));if(found!=document.metadata().end()){bool valid=false;const auto value=QString::fromStdString(found->second).toUInt(&valid);if(valid&&(value==Version2||value==Version3))return value;}}return Version3;
}
std::vector<std::uint8_t> createHeader(const Layout& layout,std::uint32_t version,std::int64_t capacity){
    using namespace Constants;std::vector<std::uint8_t> bytes(version==Version3?Header3Size:Header2Size);
    const auto u32=[&](std::size_t offset,std::uint32_t value){qToBigEndian(value,bytes.data()+offset);};
    const auto u64=[&](std::size_t offset,std::uint64_t value){qToBigEndian(value,bytes.data()+offset);};
    u32(SignatureOffset,Magic);u32(VersionOffset,version);u32(ClusterBitsOffset,DefaultClusterBits);u64(VirtualSizeOffset,std::uint64_t(capacity));u32(L1SizeOffset,std::uint32_t(layout.l2Tables));u64(L1Offset,layout.l1Cluster*layout.clusterSize);u64(RefcountOffset,layout.refcountTableCluster*layout.clusterSize);u32(RefcountClustersOffset,std::uint32_t(layout.refcountTableClusters));
    if(version==Version3){u32(RefcountOrderOffset,DefaultRefcountOrder);u32(HeaderLengthOffset,Header3Size);}return bytes;
}
void writeContainer(const BlockImage& blocks,std::uint32_t version,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto layout=createLayout(blocks.capacity());const auto clusterSize=layout.clusterSize;std::vector<std::uint8_t> data(clusterSize);
    const auto writeAt=[&](std::uint64_t offset,std::span<const std::uint8_t> bytes){checkCancellation(token);output.seek(offset);output.write(bytes);};
    // Reserve the metadata prefix; data clusters are appended only when nonzero.
    for(std::uint64_t cluster=0;cluster<layout.dataCluster;++cluster){checkCancellation(token);output.write(data);}
    std::vector<std::uint8_t> l1(static_cast<std::size_t>(layout.l1Clusters*clusterSize));
    for(std::uint64_t table=0;table<layout.l2Tables;++table){checkCancellation(token);qToBigEndian(((layout.l2Cluster+table)*clusterSize)|CopiedFlag,l1.data()+std::size_t(table)*EntryBytes);}
    writeAt(layout.l1Cluster*clusterSize,l1);std::uint64_t allocated{};const auto entries=clusterSize/EntryBytes;
    for(std::uint64_t table=0;table<layout.l2Tables;++table){
        std::vector<std::uint8_t> l2(clusterSize);
        for(std::uint64_t index=0;index<entries;++index){
            checkCancellation(token);const auto logical=table*entries+index;if(logical>=layout.virtualClusters)break;std::fill(data.begin(),data.end(),0);
            const auto address=std::int64_t(logical*clusterSize),length=std::min(std::int64_t(clusterSize),blocks.capacity()-address);blocks.readExactly(address,std::span(data).first(std::size_t(length)),token);auto entry=ZeroFlag;
            if(!std::all_of(data.begin(),data.begin()+std::size_t(length),[](auto value){return value==0;})){const auto offset=(layout.dataCluster+allocated++)*clusterSize;entry=offset|CopiedFlag;writeAt(offset,data);}
            qToBigEndian(entry,l2.data()+std::size_t(index)*EntryBytes);
        }
        writeAt((layout.l2Cluster+table)*clusterSize,l2);
    }
    const auto used=layout.dataCluster+allocated;
    std::vector<std::uint8_t> table(static_cast<std::size_t>(layout.refcountTableClusters*clusterSize)),references(static_cast<std::size_t>(layout.refcountBlocks*clusterSize));
    for(std::uint64_t block=0;block<layout.refcountBlocks;++block){checkCancellation(token);qToBigEndian((layout.refcountBlocksCluster+block)*clusterSize,table.data()+std::size_t(block)*EntryBytes);}
    for(std::uint64_t cluster=0;cluster<used;++cluster){checkCancellation(token);qToBigEndian(std::uint16_t(1),references.data()+std::size_t(cluster)*RefcountBytes);}
    writeAt(layout.refcountTableCluster*clusterSize,table);writeAt(layout.refcountBlocksCluster*clusterSize,references);writeAt(0,createHeader(layout,version,blocks.capacity()));output.seek(used*clusterSize);
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension),std::string(AlternateExtension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&blocks->logicalBlockSize()==Constants::SectorSize&&blocks->hasCompleteReadableCoverage();}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token]{checkCancellation(token);const auto extension=output.extension().u8string();if(!document||!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::QcowWrite);const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());const auto version=selectVersion(*document);writeAtomicSeekableImage(output,[&](const auto& stream,auto stop){writeContainer(blocks,version,stream,stop);},token);return std::vector<std::filesystem::path>{output};});}
}
