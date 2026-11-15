#include "Internal.h"
#include "../Adf/AdfImages.h"
#include "../BbcDfs/BbcDfsImages.h"
namespace RetroMediaLab::MediaEngine::CpcDsk {
namespace {
std::uint8_t sizeCode(std::size_t size){for(std::uint8_t code=0;code<=Constants::SectorSizeCodeMask;++code)if(Internal::nominalSize(code)==size)return code;throw MediaError(DiagnosticCode::CpcDskSizeCode,{std::to_string(size)});}
std::uint8_t addressByte(std::int64_t value,const MediaFileSystems::SectorAddress& a){if(value<0||value>std::numeric_limits<std::uint8_t>::max())throw MediaError(DiagnosticCode::CpcDskRecordedAddress,{std::to_string(a.cylinder),std::to_string(a.head),std::to_string(a.number)});return std::uint8_t(value);}
Sector sector(const MediaFileSystems::SectorBlock& block,bool native,std::optional<std::uint32_t> adfSize){
    using namespace Constants;const auto code=block.recordedSizeCode?*block.recordedSizeCode:block.formatCode&&Internal::nominalSize(*block.formatCode)==block.data.size()?*block.formatCode:sizeCode(adfSize?std::size_t(*adfSize):block.data.size());
    const bool dataError=block.dataIntegrityValid==false||(block.integrityValid==false&&block.headerIntegrityValid!=false);auto status1=std::uint8_t(block.headerIntegrityValid==false||dataError?DataErrorMask:0),status2=std::uint8_t((dataError?DataErrorMask:0)|(block.deletedDataMark==true?DeletedDataMarkMask:0));auto data=block.data;
    if(native&&block.tag&&!block.tag->empty()){if(block.tag->size()<NativeMetadataLength)throw MediaError(DiagnosticCode::CpcDskNativeMetadata);status1=(*block.tag)[NativeStatus1Offset];status2=(*block.tag)[NativeStatus2Offset];if(block.tag->size()>NativeMetadataLength)data.assign(block.tag->begin()+NativeMetadataLength,block.tag->end());}
    const auto a=block.recordedAddress.value_or(block.address);return {addressByte(a.cylinder,a),addressByte(a.head,a),addressByte(a.number,a),code,status1,status2,std::move(data)};
}
}
Image buildImage(std::shared_ptr<const MediaFileSystems::SectorImage> source,ContainerKind kind,std::stop_token token){
    if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);const auto& geometry=source->geometry();if(geometry.cylinders>Constants::MaximumCylinderCount||geometry.heads>Constants::MaximumHeadCount)throw MediaError(DiagnosticCode::CpcDskBuilderGeometry);
    const bool native=Primitives::asciiIdentifierEqualsIgnoreCase(geometry.formatId,Constants::FormatId);std::optional<std::uint32_t> adfSize;for(const auto& profile:Adf::acornGeometries())if(Primitives::asciiIdentifierEqualsIgnoreCase(profile.geometry.formatId,geometry.formatId)){adfSize=profile.geometry.blockSize;break;}if(!adfSize)if(const auto bbc=BbcDfs::findGeometry(geometry.formatId))adfSize=bbc->blockSize;
    std::vector<std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;source->visitAvailableBlocks([&](auto number){checkCancellation(token);if(auto block=source->block(number,token))blocks.push_back(std::move(block));return true;},token);std::vector<Track> tracks;tracks.reserve(std::size_t(geometry.cylinders)*geometry.heads);
    for(std::uint32_t cylinder=0;cylinder<geometry.cylinders;++cylinder)for(std::uint32_t head=0;head<geometry.heads;++head){
        checkCancellation(token);std::vector<std::shared_ptr<const MediaFileSystems::SectorBlock>> selected;for(const auto& block:blocks)if(block->address.cylinder==cylinder&&block->address.head==head)selected.push_back(block);
        const auto order=[](const auto& block){if(block->recordedTrackOrder)return std::int64_t(*block->recordedTrackOrder);if(block->logicalBlock>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::CpcDskOverflow);return std::int64_t(block->logicalBlock);};std::stable_sort(selected.begin(),selected.end(),[&](const auto& a,const auto& b){return order(a)<order(b);});
        std::vector<Sector> sectors;std::vector<std::pair<std::uint8_t,std::size_t>> frequencies;for(const auto& block:selected){auto value=sector(*block,native,adfSize);const auto found=std::find_if(frequencies.begin(),frequencies.end(),[&](const auto& f){return f.first==value.sizeCode;});if(found==frequencies.end())frequencies.emplace_back(value.sizeCode,1);else ++found->second;sectors.push_back(std::move(value));}
        const auto code=frequencies.empty()?std::uint8_t(0):std::max_element(frequencies.begin(),frequencies.end(),[](const auto& a,const auto& b){return a.second<b.second;})->first;tracks.push_back({std::int32_t(tracks.size()),!sectors.empty(),std::uint8_t(cylinder),std::uint8_t(head),code,Constants::DefaultGap3Length,Constants::DefaultFillerByte,std::move(sectors)});
    }
    return {kind,std::uint8_t(geometry.cylinders),std::uint8_t(geometry.heads),std::move(tracks),std::move(source)};
}
}
