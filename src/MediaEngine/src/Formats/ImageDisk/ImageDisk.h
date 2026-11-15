#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
#include <map>
namespace RetroMediaLab::MediaEngine::ImageDisk {
enum class Mode:std::uint8_t {Fm500,Fm300,Fm250,Mfm500,Mfm300,Mfm250};
enum class RecordType:std::uint8_t {Unavailable,Normal,Compressed,Deleted,CompressedDeleted,NormalError,CompressedError,DeletedError,CompressedDeletedError};
enum class Section {TrackHeader,SectorNumberMap,CylinderMap,HeadMap,SectorSizeMap,SectorRecord,CompressedValue,SectorData};
struct Sector {std::uint8_t cylinder=0,head=0,number=0;std::int32_t size=0;RecordType recordType=RecordType::Unavailable;std::vector<std::uint8_t> data;};
struct Track {Mode mode=Mode::Mfm250;std::uint8_t cylinder=0,head=0;std::vector<Sector> sectors;};
struct Image {std::string comment;std::vector<Track> tracks;std::shared_ptr<const MediaFileSystems::SectorImage> sectorImage;};
using Metadata=std::map<std::string,std::string>;
RETROMEDIALAB_MEDIAENGINE_API bool hasData(RecordType) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool compressed(RecordType) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool integrityValid(RecordType) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool deleted(RecordType) noexcept;
RETROMEDIALAB_MEDIAENGINE_API RecordType recordType(bool compressed,bool valid,bool deleted) noexcept;
RETROMEDIALAB_MEDIAENGINE_API Image readDetailed(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> buildImage(std::span<const Track>);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> build(const Image&);
RETROMEDIALAB_MEDIAENGINE_API void write(const Image&,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Image buildEpson(std::shared_ptr<const MediaFileSystems::SectorImage>,Mode=Mode::Mfm250);
RETROMEDIALAB_MEDIAENGINE_API Metadata metadata(const Image&);
RETROMEDIALAB_MEDIAENGINE_API std::optional<Image> restore(std::shared_ptr<const MediaFileSystems::SectorImage>,const Metadata&);
}
