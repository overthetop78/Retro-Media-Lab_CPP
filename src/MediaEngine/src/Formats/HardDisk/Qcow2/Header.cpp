#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Qcow2 {
void validateCluster(std::uint64_t offset,const Header& header,std::int64_t length,std::string_view structure){
    if(offset>std::uint64_t(length)||offset%header.clusterSize||header.clusterSize>std::uint64_t(length)-offset)throw MediaError(DiagnosticCode::QcowClusterRange,{std::string(structure)});
}
std::uint64_t clusterOffset(std::uint64_t entry,std::uint32_t clusterSize,std::string_view structure){
    if(entry&Constants::CompressedFlag)throw MediaError(DiagnosticCode::QcowCompressed,{std::string(structure)});
    const auto offset=entry&Constants::ClusterOffsetMask;
    if(!offset||offset%clusterSize)throw MediaError(DiagnosticCode::QcowClusterOffset,{std::string(structure)});return offset;
}
Header readHeader(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()<Header2Size)throw MediaError(DiagnosticCode::QcowHeader);
    const auto bytes=context.readAsync(0,std::int32_t(std::min(std::uint64_t(context.length()),std::uint64_t(Header3Size))),token).get();
    const auto u32=[&](std::size_t offset){return qFromBigEndian<std::uint32_t>(bytes.data()+offset);};
    const auto u64=[&](std::size_t offset){return qFromBigEndian<std::uint64_t>(bytes.data()+offset);};
    if(u32(SignatureOffset)!=Magic)throw MediaError(DiagnosticCode::QcowSignature);Header result;result.version=u32(VersionOffset);
    if(result.version!=Version2&&result.version!=Version3)throw MediaError(DiagnosticCode::QcowVersion,{std::to_string(result.version)});
    if(u64(BackingOffset)||u32(BackingSizeOffset))throw MediaError(DiagnosticCode::QcowParent);
    const auto bits=u32(ClusterBitsOffset);if(bits<MinimumClusterBits||bits>MaximumClusterBits)throw MediaError(DiagnosticCode::QcowClusterBits);result.clusterSize=std::uint32_t(1)<<bits;
    const auto size=u64(VirtualSizeOffset);if(!size||size>MaximumSignedLength||size%SectorSize)throw MediaError(DiagnosticCode::QcowSize);result.virtualSize=std::int64_t(size);
    if(u32(EncryptionOffset))throw MediaError(DiagnosticCode::QcowEncryption);
    result.l1Size=u32(L1SizeOffset);const auto l1=u64(L1Offset),refcount=u64(RefcountOffset);result.refcountClusters=u32(RefcountClustersOffset);
    if(u32(SnapshotCountOffset)||u64(SnapshotOffset))throw MediaError(DiagnosticCode::QcowSnapshots);
    if(!result.l1Size||l1>MaximumSignedLength||refcount>MaximumSignedLength||!result.refcountClusters)throw MediaError(DiagnosticCode::QcowTables);
    result.l1Offset=std::int64_t(l1);result.refcountOffset=std::int64_t(refcount);validateCluster(l1,result,context.length(),L1TableName);validateCluster(refcount,result,context.length(),RefcountTableName);
    const auto l1Bytes=std::uint64_t(result.l1Size)*EntryBytes,refBytes=std::uint64_t(result.refcountClusters)*result.clusterSize;
    if(l1Bytes>MaximumBuffer||l1Bytes>std::uint64_t(context.length())-l1||refBytes>MaximumBuffer||refBytes>std::uint64_t(context.length())-refcount)throw MediaError(DiagnosticCode::QcowTableRange);
    auto order=DefaultRefcountOrder;
    if(result.version==Version3){
        if(bytes.size()<Header3Size)throw MediaError(DiagnosticCode::QcowHeader);
        const auto features=u64(IncompatibleOffset);if(features&~KnownIncompatibleFeatures)throw MediaError(DiagnosticCode::QcowFeature,{std::to_string(features)});
        if(u64(AutoclearOffset))throw MediaError(DiagnosticCode::QcowAutoclear);order=u32(RefcountOrderOffset);const auto length=u32(HeaderLengthOffset);
        if(length<Header3Size||length>result.clusterSize)throw MediaError(DiagnosticCode::QcowHeaderLength);
    }
    if(order!=DefaultRefcountOrder)throw MediaError(DiagnosticCode::QcowRefcountOrder,{std::to_string(order)});return result;
}
}
