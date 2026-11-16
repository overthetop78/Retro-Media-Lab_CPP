#pragma once
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::Qcow2 {
struct Layout {
    std::uint64_t virtualClusters=0,l2Tables=0,l1Clusters=0,refcountBlocks=0,refcountTableClusters=0;
    std::uint64_t refcountTableCluster=0,refcountBlocksCluster=0,l1Cluster=0,l2Cluster=0,dataCluster=0;
    std::uint32_t clusterSize=0;
};
Layout createLayout(std::int64_t capacity);
}
