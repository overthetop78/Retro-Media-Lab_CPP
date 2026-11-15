#include "Internal.h"
#include "../../Models/Sectors/UniformLayout.h"
#include "../../Sources/RandomAccessSources.h"
#include <tuple>
namespace RetroMediaLab::MediaEngine::CpcDsk {
bool canRead(MediaFileSystems::Bytes header,std::uint64_t length,std::optional<std::string_view> requested){return (!requested||Primitives::asciiIdentifierEqualsIgnoreCase(*requested,Constants::FormatId))&&length>=Constants::DiskInformationBlockSize&&(Internal::starts(header,Constants::StandardSignature)||Internal::starts(header,Constants::ExtendedSignature));}
Image readDetailed(MediaFileSystems::Bytes bytes,std::stop_token token){
    using namespace Constants;using namespace Internal;if(bytes.size()<DiskInformationBlockSize)throw MediaError(DiagnosticCode::CpcDskHeader);
    const auto kind=starts(bytes.first(DiskSignatureLength),ExtendedSignature)?ContainerKind::Extended:ContainerKind::Standard;if(kind==ContainerKind::Standard&&!starts(bytes.first(DiskSignatureLength),StandardSignature))throw MediaError(DiagnosticCode::CpcDskSignature);
    const auto cylinders=bytes[CylinderCountOffset],heads=bytes[HeadCountOffset];if(cylinders<MinimumCylinderCount||cylinders>MaximumCylinderCount||heads<MinimumHeadCount||heads>MaximumHeadCount)throw MediaError(DiagnosticCode::CpcDskGeometry);const auto trackCount=std::size_t(cylinders)*heads;
    if(kind==ContainerKind::Extended&&ExtendedTrackSizeTableOffset+trackCount>DiskInformationBlockSize)throw MediaError(DiagnosticCode::CpcDskExtendedTable,{std::to_string(DiskInformationBlockSize-ExtendedTrackSizeTableOffset)});
    std::vector<MediaFileSystems::SectorBlock> blocks;std::vector<Track> tracks;tracks.reserve(trackCount);std::vector<std::pair<std::uint32_t,std::size_t>> frequencies;std::size_t position=DiskInformationBlockSize,maximumSectors=0;
    for(std::size_t index=0;index<trackCount;++index){
        checkCancellation(token);const auto size=kind==ContainerKind::Extended?std::size_t(bytes[ExtendedTrackSizeTableOffset+index])*ExtendedTrackSizeUnit:word(bytes,StandardTrackSizeOffset);
        if(size==0){tracks.push_back({std::int32_t(index),false,std::uint8_t(index/heads),std::uint8_t(index%heads),0,0,0,{}});continue;}
        if(position>bytes.size()||size>bytes.size()-position||size<TrackInformationBlockSize)throw MediaError(DiagnosticCode::CpcDskTrackTruncated,{std::to_string(index)});
        if(!starts(bytes.subspan(position,TrackSignatureLength),TrackSignature))throw MediaError(DiagnosticCode::CpcDskTrackHeader,{std::to_string(index)});
        Track track{std::int32_t(index),true,bytes[position+TrackCylinderOffset],bytes[position+TrackHeadOffset],bytes[position+TrackSectorSizeCodeOffset],bytes[position+TrackGap3LengthOffset],bytes[position+TrackFillerByteOffset],{}};const auto count=bytes[position+TrackSectorCountOffset];
        if(SectorDescriptorTableOffset+std::size_t(count)*SectorDescriptorSize>TrackInformationBlockSize)throw MediaError(DiagnosticCode::CpcDskSectorTable,{std::to_string(index)});std::size_t dataPosition=position+TrackInformationBlockSize;
        for(std::size_t sectorIndex=0;sectorIndex<count;++sectorIndex){
            const auto descriptor=position+SectorDescriptorTableOffset+sectorIndex*SectorDescriptorSize;const auto c=bytes[descriptor+SectorCylinderOffset],h=bytes[descriptor+SectorHeadOffset],id=bytes[descriptor+SectorIdOffset],code=std::uint8_t(bytes[descriptor+SectorSizeCodeOffset]&SectorSizeCodeMask);const auto nominal=nominalSize(code);std::size_t stored=kind==ContainerKind::Extended?word(bytes,descriptor+SectorStoredSizeOffset):nominal;if(stored==0)stored=nominal;
            if(dataPosition>position+size||stored>position+size-dataPosition)throw MediaError(DiagnosticCode::CpcDskSectorTruncated,{std::to_string(track.cylinder),std::to_string(track.head),std::to_string(id)});
            const auto status1=bytes[descriptor+SectorStatus1Offset],status2=bytes[descriptor+SectorStatus2Offset];auto payload=bytes.subspan(dataPosition,stored);track.sectors.push_back({c,h,id,code,status1,status2,{payload.begin(),payload.end()}});
            MediaFileSystems::SectorBlock block;block.logicalBlock=blocks.size();block.address={track.cylinder,track.head,id};block.data.assign(payload.begin(),payload.begin()+std::min(std::size_t(nominal),stored));block.integrityValid=(status1&DataErrorMask)==0&&(status2&DataErrorMask)==0;
            if(status1!=0||status2!=0||stored>nominal){block.tag=std::vector<std::uint8_t>{status1,status2};if(stored>nominal)block.tag->insert(block.tag->end(),payload.begin(),payload.end());}
            block.formatCode=code;block.recordedAddress=MediaFileSystems::SectorAddress{c,h,id};block.recordedTrackOrder=std::int32_t(sectorIndex);block.recordedSizeCode=code;block.headerIntegrityValid=(status1&DataErrorMask)==0||(status2&DataErrorMask)!=0;block.dataIntegrityValid=(status2&DataErrorMask)==0;block.deletedDataMark=(status2&DeletedDataMarkMask)!=0;blocks.push_back(std::move(block));
            const auto frequency=std::find_if(frequencies.begin(),frequencies.end(),[&](const auto& value){return value.first==nominal;});if(frequency==frequencies.end())frequencies.emplace_back(nominal,1);else ++frequency->second;dataPosition+=stored;
        }
        maximumSectors=std::max(maximumSectors,track.sectors.size());tracks.push_back(std::move(track));position+=size;
    }
    if(blocks.empty())throw MediaError(DiagnosticCode::CpcDskNoSectors);std::stable_sort(blocks.begin(),blocks.end(),[](const auto& a,const auto& b){return std::tie(a.address.cylinder,a.address.head,a.address.number)<std::tie(b.address.cylinder,b.address.head,b.address.number);});for(std::size_t index=0;index<blocks.size();++index)blocks[index].logicalBlock=index;
    const auto dominant=std::max_element(frequencies.begin(),frequencies.end(),[](const auto& a,const auto& b){return a.second<b.second;})->first;MediaFileSystems::SectorGeometry geometry{std::string(FormatId),dominant,cylinders,heads,std::uint32_t(std::max(std::size_t(MinimumSectorsPerTrack),maximumSectors))};std::shared_ptr<const MediaFileSystems::SectorImage> image;if(frequencies.size()==1)image=tryUniformSectorImage(geometry,blocks,token);
    if(!image){MediaFileSystems::SectorImageOptions options;options.allowVariableBlockSize=frequencies.size()>1;options.logicalBlockCount=blocks.size();std::uint64_t capacity=0;for(const auto& block:blocks)capacity+=block.data.size();options.capacity=capacity;image=sparseSectorImage(std::move(geometry),std::move(blocks),options);}return {kind,cylinders,heads,std::move(tracks),std::move(image)};
}
Image readDetailed(const std::filesystem::path& path,std::stop_token token){checkCancellation(token);const auto source=fileSource(path);if(source->length()>Constants::MaximumBuffer)throw MediaError(DiagnosticCode::CpcDskOverflow);std::vector<std::uint8_t> bytes(std::size_t(source->length()));source->readExactly(0,bytes,token);return readDetailed(bytes,token);}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes bytes,std::stop_token token){return readDetailed(bytes,token).sectorImage;}
std::shared_ptr<const MediaFileSystems::SectorImage> read(const std::filesystem::path& path,std::stop_token token){return readDetailed(path,token).sectorImage;}
}
