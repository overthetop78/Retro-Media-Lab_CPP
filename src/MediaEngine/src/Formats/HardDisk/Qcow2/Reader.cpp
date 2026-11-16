#include "Refcounts.h"
#include "../../../Images/ReadRejection.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <map>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Qcow2 {
namespace {
std::shared_ptr<const MediaDocument> readDocument(const RecognitionContext& context,bool includeRanges,std::stop_token token){
    using namespace Constants;const auto header=readHeader(context,token);Refcounts refs(context,header,token);refs.requireAllocated(0,token);
    const auto l1=context.readAsync(header.l1Offset,std::int32_t(std::uint64_t(header.l1Size)*EntryBytes),token).get();
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source;
    if(includeRanges){const auto& path=context.source()->primaryPath();source=fileSource(std::filesystem::path(std::u8string(path.begin(),path.end())));}
    const auto clusters=(std::uint64_t(header.virtualSize)+header.clusterSize-1)/header.clusterSize,entries=header.clusterSize/EntryBytes;
    std::vector<DataRange> ranges;std::map<std::uint64_t,std::vector<std::uint8_t>> tables;
    for(std::uint64_t cluster=0;cluster<clusters;++cluster){
        checkCancellation(token);const auto index=cluster/entries;if(index>=header.l1Size)throw MediaError(DiagnosticCode::QcowL1Coverage);
        const auto entry=qFromBigEndian<std::uint64_t>(l1.data()+std::size_t(index)*EntryBytes);const auto address=std::int64_t(cluster*header.clusterSize),length=std::min(std::int64_t(header.clusterSize),header.virtualSize-address);
        if(!entry){addBlockRange(ranges,address,length,DataRangeKind::Unallocated);continue;}
        const auto offset=clusterOffset(entry,header.clusterSize,L1Name);refs.requireAllocated(offset,token);
        auto found=tables.find(offset);if(found==tables.end())found=tables.emplace(offset,context.readAsync(std::int64_t(offset),std::int32_t(header.clusterSize),token).get()).first;
        const auto dataEntry=qFromBigEndian<std::uint64_t>(found->second.data()+std::size_t(cluster%entries)*EntryBytes);
        if(dataEntry&CompressedFlag)throw MediaError(DiagnosticCode::QcowCompressed,{std::string(L2DataName)});
        if(dataEntry&ZeroFlag){addBlockRange(ranges,address,length,DataRangeKind::Zero);continue;}
        const auto dataOffset=dataEntry&ClusterOffsetMask;if(!dataOffset){addBlockRange(ranges,address,length,DataRangeKind::Unallocated);continue;}
        validateCluster(dataOffset,header,context.length(),L2DataName);refs.requireAllocated(dataOffset,token);
        addBlockRange(ranges,address,length,includeRanges?DataRangeKind::Stored:DataRangeKind::Unavailable,source,includeRanges?std::int64_t(dataOffset):0);
    }
    auto blocks=std::make_shared<const BlockImage>(header.virtualSize,SectorSize,std::move(ranges));
    return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::HardDisk,std::move(blocks),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VersionKey),std::to_string(header.version)},{std::string(VariantKey),std::string(SparseVariant)},{std::string(ClusterSizeKey),std::to_string(header.clusterSize)}});
}
}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension),std::string(AlternateExtension)},{std::vector<std::uint8_t>(Signature.begin(),Signature.end())},{},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{readDocument(context,false,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{return readDocument(context,true,token);});}
}
