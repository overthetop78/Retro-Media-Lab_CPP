#include "Refcounts.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::Qcow2 {
Refcounts::Refcounts(RecognitionContext context,const Header& header,std::stop_token token):context_(std::move(context)),header_(header),table_(context_.readAsync(header.refcountOffset,std::int32_t(std::uint64_t(header.refcountClusters)*header.clusterSize),token).get()){}
void Refcounts::requireAllocated(std::uint64_t offset,std::stop_token token){
    using namespace Constants;checkCancellation(token);validateCluster(offset,header_,context_.length(),ReferenceName);
    const auto cluster=offset/header_.clusterSize,entries=header_.clusterSize/RefcountBytes,index=cluster/entries;
    if(index>=table_.size()/EntryBytes)throw MediaError(DiagnosticCode::QcowRefCoverage);
    const auto blockOffset=qFromBigEndian<std::uint64_t>(table_.data()+std::size_t(index)*EntryBytes)&ClusterOffsetMask;
    if(!blockOffset)throw MediaError(DiagnosticCode::QcowRefMissing);validateCluster(blockOffset,header_,context_.length(),RefcountBlockName);
    auto found=blocks_.find(blockOffset);if(found==blocks_.end())found=blocks_.emplace(blockOffset,context_.readAsync(std::int64_t(blockOffset),std::int32_t(header_.clusterSize),token).get()).first;
    if(!qFromBigEndian<std::uint16_t>(found->second.data()+std::size_t(cluster%entries)*RefcountBytes))throw MediaError(DiagnosticCode::QcowRefZero);
}
}
