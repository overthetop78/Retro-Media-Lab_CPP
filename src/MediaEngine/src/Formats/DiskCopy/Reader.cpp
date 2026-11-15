#include "DiskCopy.h"
#include "Constants.h"
#include "../Apple/Raw.h"
#include "../Apple/Geometry.h"
#include "../FormatIds.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <bit>
namespace RetroMediaLab::MediaEngine::DiskCopy {
namespace {
using namespace Constants;
namespace G=Apple::Geometry;
std::uint32_t big(Bytes bytes,std::size_t offset,std::size_t size){
    std::uint32_t value=0;for(std::size_t i=0;i<size;++i)value=(value<<BitsPerByte)|bytes[offset+i];return value;
}
enum class Kind {Lisa,Mac400,Mac800,Generic};
Kind kind(std::size_t count){
    if(count==G::LisaFileWare::BlockCount)return Kind::Lisa;
    if(count==G::MacintoshGcr::SingleSidedBlockCount)return Kind::Mac400;
    if(count==G::MacintoshGcr::SingleSidedBlockCount*G::DoubleHeads)return Kind::Mac800;
    return Kind::Generic;
}
MediaFileSystems::SectorAddress address(Kind layout,std::int32_t logical){
    if(layout==Kind::Lisa)return G::LisaFileWare::address(logical);
    if(layout==Kind::Mac400)return G::MacintoshGcr::address(logical,G::SingleHead);
    if(layout==Kind::Mac800)return G::MacintoshGcr::address(logical,G::DoubleHeads);
    return {logical/G::Tagged::SectorsPerTrack,G::Tagged::FirstHead,logical%G::Tagged::SectorsPerTrack};
}
MediaFileSystems::SectorGeometry geometry(Kind layout,std::size_t count,std::string id){
    if(layout==Kind::Lisa)return {std::move(id),DataBlockSize,G::LisaFileWare::CylinderCount,G::DoubleHeads,G::LisaFileWare::MaximumSectorsPerTrack};
    if(layout==Kind::Mac400||layout==Kind::Mac800)return {std::move(id),DataBlockSize,G::MacintoshGcr::CylinderCount,std::uint32_t(layout==Kind::Mac400?G::SingleHead:G::DoubleHeads),G::MacintoshGcr::MaximumSectorsPerTrack};
    return {std::move(id),DataBlockSize,std::uint32_t(std::max(std::size_t(G::Tagged::MinimumCylinderCount),count/G::Tagged::SectorsPerTrack)),G::Tagged::HeadCount,G::Tagged::SectorsPerTrack};
}
std::string format(Bytes payload,Kind layout,std::uint8_t formatByte){
    if((layout==Kind::Mac400||layout==Kind::Mac800)&&Apple::probeMacintosh(payload))return std::string(formatByte==FormatMfs?Formats::AppleMacMfs:Formats::AppleMacHfs);
    constexpr auto offset=PrebootSearchBlockIndex*DataBlockSize;
    if(payload.size()>=offset+PrebootSearchLength){
        const auto window=payload.subspan(offset,PrebootSearchLength);
        if(std::search(window.begin(),window.end(),PrebootMarker.begin(),PrebootMarker.end())!=window.end())return std::string(AppleLisaMacWorks);
    }
    return std::string(AppleLisaOffice);
}
}
bool hasPrivateWord(Bytes bytes) noexcept{return bytes.size()>=Constants::HeaderSize&&big(bytes,Constants::PrivateWordOffset,sizeof(Constants::PrivateWord))==Constants::PrivateWord;}
std::uint32_t checksum(Bytes bytes){
    if(bytes.size()%Constants::ChecksumWordSize)throw MediaError(DiagnosticCode::DiskCopyChecksumBytes,{std::to_string(bytes.size())});
    auto value=Constants::InitialChecksum;
    for(std::size_t offset=0;offset<bytes.size();offset+=Constants::ChecksumWordSize)value=std::rotr(std::uint32_t(value+big(bytes,offset,Constants::ChecksumWordSize)),Constants::ChecksumRotation);
    return value;
}
Image readDetailed(Bytes container,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    if(container.size()<HeaderSize)throw MediaError(DiagnosticCode::DiskCopyHeader);
    const auto word=big(container,PrivateWordOffset,sizeof(PrivateWord));
    if(word!=PrivateWord)throw MediaError(DiagnosticCode::DiskCopyPrivateWord,{std::to_string(word),std::to_string(PrivateWord)});
    const auto dataLength=big(container,DataLengthOffset,sizeof(std::uint32_t)),tagLength=big(container,TagLengthOffset,sizeof(std::uint32_t));
    if(dataLength>MaximumLength||tagLength>MaximumLength)throw MediaError(DiagnosticCode::DiskCopyRange);
    if(dataLength<MinimumDataLength||std::uint64_t(HeaderSize)+dataLength+tagLength>container.size())throw MediaError(DiagnosticCode::DiskCopyPayload);
    const auto payload=container.subspan(HeaderSize,dataLength),tags=container.subspan(HeaderSize+dataLength,tagLength);
    const auto storedData=big(container,DataChecksumOffset,sizeof(std::uint32_t)),storedTags=big(container,TagChecksumOffset,sizeof(std::uint32_t));
    std::optional<bool> dataValid,tagValid;
    if(storedData!=MissingChecksum)dataValid=storedData==checksum(payload);
    if(storedTags!=MissingChecksum)tagValid=tags.size()>=TagChecksumExcludedPrefixSize&&storedTags==checksum(tags.subspan(TagChecksumExcludedPrefixSize));
    std::shared_ptr<const SectorImage> image;
    if(tagLength==0&&(Apple::probeMacintosh(payload)||Apple::looksLikeProDos(payload)))image=Apple::readRaw(payload,ImageExtension,token).image;
    else {
        if(tagLength!=dataLength/DataBlockSize*TagSize)throw MediaError(DiagnosticCode::DiskCopyTags);
        const auto count=dataLength/DataBlockSize;const auto layout=kind(count);
        std::vector<MediaFileSystems::SectorBlock> blocks;blocks.reserve(count);
        for(std::uint32_t logical=0;logical<count;++logical){
            checkCancellation(token);MediaFileSystems::SectorBlock block;block.logicalBlock=logical;block.address=address(layout,std::int32_t(logical));
            const auto bytes=payload.subspan(logical*DataBlockSize,DataBlockSize),tag=tags.subspan(logical*TagSize,TagSize);
            block.data.assign(bytes.begin(),bytes.end());block.tag=std::vector<std::uint8_t>(tag.begin(),tag.end());blocks.push_back(std::move(block));
        }
        MediaFileSystems::SectorImageOptions options;options.capacity=dataLength;options.logicalBlockCount=count;
        image=sparseSectorImage(geometry(layout,count,format(payload,layout,container[FormatByteOffset])),std::move(blocks),options);
    }
    const auto length=std::min(std::size_t(container[NameLengthOffset]),MaximumNameLength);
    const auto name=container.subspan(NameOffset,length);
    return {std::move(image),{name.begin(),name.end()},container[DiskFormatOffset],container[FormatByteOffset],dataValid,tagValid};
}
std::shared_ptr<const SectorImage> read(Bytes container,std::stop_token token){return readDetailed(container,token).image;}
}
