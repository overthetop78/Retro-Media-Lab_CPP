#pragma once
#include "Header.h"
#include "../../../Sources/AtomicImageWriter.h"
#include "../../../Models/Blocks/BlockImage.h"
#include <map>
namespace RetroMediaLab::MediaEngine::MfmEmu {
class Container final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const Container> open(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const Container> blank(Header);
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const Container> formatted(Header,HardDiskGeometry,std::vector<std::uint8_t> logicalPrefix);
    const Header& header()const noexcept{return *header_;}
    std::uint64_t length()const noexcept{return source_->length();}
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Container> withTrackBytes(std::int32_t cylinder,std::int32_t head,std::span<const std::uint8_t>)const;
    RETROMEDIALAB_MEDIAENGINE_API void readTrackBytes(std::int32_t cylinder,std::int32_t head,std::int32_t byteOffset,MediaFileSystems::MutableBytes,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API void copyTo(const ImageOutput&,std::stop_token={})const;
private:
    using Changes=std::map<std::uint64_t,std::shared_ptr<const std::vector<std::uint8_t>>>;
    Container(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::shared_ptr<const Header> header,Changes changes={}):source_(std::move(source)),header_(std::move(header)),changes_(std::move(changes)){}
    void validateTrack(std::uint64_t,std::int32_t,std::int32_t,std::stop_token)const;
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;std::shared_ptr<const Header> header_;Changes changes_;
};
std::shared_ptr<const MediaFileSystems::RandomAccessData> blankData(std::shared_ptr<const Header>);
std::shared_ptr<const MediaFileSystems::RandomAccessData> formattedData(std::shared_ptr<const Header>,HardDiskGeometry,std::vector<std::uint8_t>);
}
