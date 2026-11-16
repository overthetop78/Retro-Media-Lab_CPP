#include "Layout.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Qcow2 {
namespace {
std::uint64_t rounded(std::uint64_t value,std::uint64_t unit){return value/unit+(value%unit!=0);}
}
Layout createLayout(std::int64_t capacity){
    using namespace Constants;if(capacity<=0||capacity%SectorSize)throw MediaError(DiagnosticCode::QcowLayout);Layout result;
    result.clusterSize=std::uint32_t(1)<<DefaultClusterBits;result.virtualClusters=rounded(std::uint64_t(capacity),result.clusterSize);result.l2Tables=rounded(result.virtualClusters,result.clusterSize/EntryBytes);
    if(result.l2Tables>MaximumField)throw MediaError(DiagnosticCode::QcowLayout);result.l1Clusters=rounded(result.l2Tables*EntryBytes,result.clusterSize);
    result.refcountBlocks=result.refcountTableClusters=1;
    for(;;){
        const auto maximum=1+result.refcountTableClusters+result.refcountBlocks+result.l1Clusters+result.l2Tables+result.virtualClusters;
        const auto blocks=rounded(maximum,result.clusterSize/RefcountBytes),table=rounded(blocks*EntryBytes,result.clusterSize);
        if(blocks==result.refcountBlocks&&table==result.refcountTableClusters)break;result.refcountBlocks=blocks;result.refcountTableClusters=table;
    }
    if(result.refcountTableClusters>MaximumField||result.refcountTableClusters*result.clusterSize>MaximumBuffer||result.refcountBlocks*result.clusterSize>MaximumBuffer||result.l1Clusters*result.clusterSize>MaximumBuffer)throw MediaError(DiagnosticCode::QcowLayout);
    result.refcountTableCluster=1;result.refcountBlocksCluster=1+result.refcountTableClusters;result.l1Cluster=result.refcountBlocksCluster+result.refcountBlocks;result.l2Cluster=result.l1Cluster+result.l1Clusters;result.dataCluster=result.l2Cluster+result.l2Tables;
    if(result.dataCluster+result.virtualClusters>MaximumSignedLength/result.clusterSize||(result.dataCluster+result.virtualClusters-1)>ClusterOffsetMask/result.clusterSize)throw MediaError(DiagnosticCode::QcowLayout);return result;
}
}
