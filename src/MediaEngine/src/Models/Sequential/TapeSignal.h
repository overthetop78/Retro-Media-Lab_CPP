#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>
#include <map>
#include <string>
namespace RetroMediaLab::MediaEngine {
class MediaTapeBlock final {
public:
    RETROMEDIALAB_MEDIAENGINE_API MediaTapeBlock(int samplePosition,std::vector<std::uint8_t>,std::optional<bool>,std::map<std::string,std::string>);
    int samplePosition()const noexcept{return position_;}
    std::span<const std::uint8_t> data()const noexcept{return data_?std::span<const std::uint8_t>(*data_):std::span<const std::uint8_t>{};}
    std::optional<bool> integrityValid()const noexcept{return integrity_;}
    const std::map<std::string,std::string>& metadata()const noexcept{return metadata_;}
private:
    int position_;
    std::shared_ptr<const std::vector<std::uint8_t>> data_;
    std::optional<bool> integrity_;
    std::map<std::string,std::string> metadata_;
};
class MediaTapeSignal final {
public:
    RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal(std::vector<std::int16_t>,int sampleRate,std::optional<std::vector<int>> positions={},std::optional<std::vector<MediaTapeBlock>> blocks={});
    std::span<const std::int16_t> samples()const noexcept{return samples_?std::span<const std::int16_t>(*samples_):std::span<const std::int16_t>{};}
    int sampleRate()const noexcept{return rate_;}
    const std::optional<std::vector<int>>& blockPositions()const noexcept{return positions_;}
    const std::optional<std::vector<MediaTapeBlock>>& blocks()const noexcept{return blocks_;}
    RETROMEDIALAB_MEDIAENGINE_API MediaTapeSignal withBlocks(std::vector<MediaTapeBlock>,std::optional<std::vector<int>> positions)const;
private:
    std::shared_ptr<const std::vector<std::int16_t>> samples_;
    int rate_;
    std::optional<std::vector<int>> positions_;
    std::optional<std::vector<MediaTapeBlock>> blocks_;
};
}
