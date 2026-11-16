#pragma once
#include "ContainerConstants.h"
#include "Compression.h"
#include "Huffman.h"
#include "Flac.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <array>
#include <mutex>
#include <optional>
#include <string>
namespace RetroMediaLab::MediaEngine::Chd {
// Owns the source and codec workspaces. Public reads serialize workspace access.
class Container final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Container(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
    std::uint32_t version()const noexcept{return version_;}
    std::uint32_t hunkBytes()const noexcept{return hunkBytes_;}
    std::uint32_t unitBytes()const noexcept{return unitBytes_;}
    std::uint32_t hunkCount()const noexcept{return hunkCount_;}
    std::uint64_t logicalBytes()const noexcept{return logicalBytes_;}
    std::uint64_t metadataOffset()const noexcept{return metadataOffset_;}
    RETROMEDIALAB_MEDIAENGINE_API std::string compression()const;
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> metadata(std::string_view,int,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API void readHunk(std::uint32_t,std::span<std::uint8_t>,std::stop_token={});
private:
    struct Entry {std::uint64_t offset=0;std::uint32_t length=0;std::uint8_t type=0;};
    void readAt(std::uint64_t,std::span<std::uint8_t>,std::stop_token)const;
    void readHeader(std::stop_token);void readLegacyMap(std::uint64_t,std::stop_token);void readMap5(std::uint64_t,std::stop_token);
    void initializeCodecs();void decompress(std::uint32_t,std::span<const std::uint8_t>,std::span<std::uint8_t>,std::stop_token);
    void reassembleCd(std::span<std::uint8_t>,std::span<const std::uint8_t>,std::stop_token);
    std::span<const std::uint8_t> readCompressed(std::uint64_t,std::uint32_t,std::stop_token);
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;
    std::uint32_t version_=0,hunkBytes_=0,unitBytes_=0,hunkCount_=0;
    std::uint64_t logicalBytes_=0,metadataOffset_=0;
    std::array<std::uint32_t,Constants::CodecCount> codecs_{};
    std::vector<std::uint8_t> map_,compressed_,cdBuffer_;
    std::vector<Entry> legacyMap_;
    Compression codecsWorkspace_;std::unique_ptr<Huffman> huffman_;Flac flac_;mutable std::mutex mutex_;
};
}
