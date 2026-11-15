#pragma once
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <optional>
namespace RetroMediaLab::MediaEngine::TeleDisk {
struct Header {std::uint8_t sequence{},checkSignature{},version{},dataRate{},driveType{},trackDensity{},dosMode{},surfaces{};};
struct Comment {std::uint8_t year{},month{},day{},hour{},minute{},second{};std::vector<std::uint8_t> data;};
struct Sector {std::uint8_t cylinder{},head{},number{},sizeCode{},flags{};std::optional<std::vector<std::uint8_t>> data;};
struct Track {std::uint8_t cylinder{},head{};std::vector<Sector> sectors;};
struct Image {Header header;std::optional<Comment> comment;std::vector<Track> tracks;std::shared_ptr<const MediaFileSystems::SectorImage> sectorImage;};
enum class SectorEncoding:std::uint8_t {Raw=0,RepeatedWord=1,Rle=2};
struct EncodedSector {SectorEncoding encoding{};std::vector<std::uint8_t> payload;};
}
