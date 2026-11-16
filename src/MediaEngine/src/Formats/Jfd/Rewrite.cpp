#include "Jfd.h"
#include "Binary.h"
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaEngine::Jfd {
std::vector<std::uint8_t> rewrite(const Image& native,const MediaFileSystems::SectorImage& sectors,std::stop_token token) {
    using namespace Constants;using Key=std::pair<std::int64_t,std::int32_t>;auto bytes=native.bytes;
    std::map<Key,std::shared_ptr<const MediaFileSystems::SectorBlock>> supplied;
    sectors.visitAvailableBlocks([&](auto logical){auto block=MediaFileSystems::requireSectorBlock(sectors,logical,token);const Key key{block->address.cylinder*Heads+block->address.head,block->recordedTrackOrder.value_or(MissingOrder)};
        if(!supplied.emplace(key,std::move(block)).second)throw MediaError(DiagnosticCode::JfdDuplicateEdit);return true;},token);
    std::map<std::size_t,std::vector<std::uint8_t>> writes;std::map<std::size_t,std::uint32_t> headers;
    for(const auto& record:native.records){checkCancellation(token);if(record.rawTrack()||!record.offset)continue;
        const auto found=supplied.find({record.track,std::int32_t(record.order)});if(found==supplied.end())throw MediaError(DiagnosticCode::JfdEditGeometry);
        const auto& block=*found->second;
        if(block.data.size()!=record.length||block.address.number!=((record.header>>NumberShift)&ByteMask)||block.deletedDataMark==true||(block.recordedAddress&&*block.recordedAddress!=block.address)||block.recordedSizeCode!=std::uint8_t(record.header&SizeMask))throw MediaError(DiagnosticCode::JfdEditGeometry);
        auto header=record.header&~ErrorMask;
        if(!block.headerIntegrityValid.value_or((record.header&HeaderError)==0))header|=HeaderError;
        if(!block.dataIntegrityValid.value_or(block.integrityValid.value_or((record.header&DataError)==0)))header|=DataError;
        if(auto previous=headers.find(record.descriptorOffset);previous!=headers.end()&&previous->second!=header)throw MediaError(DiagnosticCode::JfdDescriptorConflict);
        headers[record.descriptorOffset]=header;
        if(auto previous=writes.find(*record.offset);previous!=writes.end()&&previous->second!=block.data)throw MediaError(DiagnosticCode::JfdDataConflict);
        writes[*record.offset]=block.data;supplied.erase(found);
    }
    if(!supplied.empty())throw MediaError(DiagnosticCode::JfdExtraSectors);
    std::size_t coveredEnd{};
    for(const auto& [offset,data]:writes){checkCancellation(token);Binary::require(bytes,offset,data.size(),bytes.size());const auto overlap=std::min(data.size(),coveredEnd>offset?coveredEnd-offset:0);
        if(!std::equal(data.begin(),data.begin()+overlap,bytes.begin()+offset))throw MediaError(DiagnosticCode::JfdOverlapConflict);
        std::copy(data.begin(),data.end(),bytes.begin()+offset);coveredEnd=std::max(coveredEnd,offset+data.size());
    }
    for(const auto& [offset,header]:headers)Binary::put(bytes,offset,header);return bytes;
}
}
