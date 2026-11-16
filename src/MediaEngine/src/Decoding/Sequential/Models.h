#pragma once
#include "../../Models/Sequential/Image.h"
#include "../../Images/MediaDocument.h"
namespace RetroMediaLab::MediaEngine {
class SequentialDecodedBlock final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialDecodedBlock(std::int64_t position,std::vector<std::uint8_t> data,std::vector<std::int64_t> sourcePositions,std::optional<bool> integrity,std::map<std::string,std::string> metadata={});
    RETROMEDIALAB_MEDIAENGINE_API SequentialDecodedBlock atPosition(std::int64_t)const;
    RETROMEDIALAB_MEDIAENGINE_API SequentialDecodedBlock withSourcePositions(std::vector<std::int64_t>)const;
    std::int64_t position()const noexcept{return position_;}
    std::span<const std::uint8_t> data()const noexcept{return data_?std::span<const std::uint8_t>(*data_):std::span<const std::uint8_t>{};}
    std::span<const std::int64_t> sourcePositions()const noexcept{return positions_;}
    std::optional<bool> integrityValid()const noexcept{return integrity_;}
    const std::map<std::string,std::string>& metadata()const noexcept{return metadata_;}
private:
    std::int64_t position_;std::shared_ptr<const std::vector<std::uint8_t>> data_;std::vector<std::int64_t> positions_;std::optional<bool> integrity_;std::map<std::string,std::string> metadata_;
};
class SequentialDecodeResult final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialDecodeResult(std::string decoderId,double confidence,std::vector<SequentialDecodedBlock> blocks,std::vector<SequentialSegment> undecoded,std::vector<std::string> diagnostics={},std::vector<MediaFileSystems::MessageRecipe> messages={});
    const std::string& decoderId()const noexcept{return decoderId_;}
    double confidence()const noexcept{return confidence_;}
    std::span<const SequentialDecodedBlock> blocks()const noexcept{return blocks_;}
    std::span<const SequentialSegment> undecodedSegments()const noexcept{return undecoded_;}
    std::span<const std::string> diagnostics()const noexcept{return diagnostics_;}
    std::span<const MediaFileSystems::MessageRecipe> messages()const noexcept{return messages_;}
private:
    std::string decoderId_;double confidence_;std::vector<SequentialDecodedBlock> blocks_;std::vector<SequentialSegment> undecoded_;std::vector<std::string> diagnostics_;std::vector<MediaFileSystems::MessageRecipe> messages_;
};
}
