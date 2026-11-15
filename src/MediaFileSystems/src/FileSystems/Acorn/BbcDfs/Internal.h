#pragma once
#include <RetroMediaLab/MediaFileSystems/BbcDfs.h>
#include "Constants.h"
namespace RetroMediaLab::MediaFileSystems::BbcDfs::Internal {
bool same(std::string_view,std::string_view);
bool twoSided(const SectorImage&);
bool validIntegrity(const SectorBlock&);
class CatalogueView {
public:
    explicit CatalogueView(const SectorImage&,std::optional<std::uint32_t> head = {});
    const SectorGeometry& geometry()const noexcept{return geometry_;}
    std::uint64_t blockCount()const noexcept{return count_;}
    std::uint64_t capacity()const noexcept{return count_*geometry_.blockSize;}
    std::shared_ptr<const SectorBlock> block(std::uint64_t,std::stop_token)const;
private:
    const SectorImage& source_;
    std::optional<std::uint32_t> head_;
    SectorGeometry geometry_;
    std::uint64_t count_;
};
bool canReadSingle(const CatalogueView&,std::stop_token);
FileSystemVolume readSingle(const CatalogueView&,std::stop_token);
FileSystemVolume readSides(const SectorImage&,std::stop_token);
bool canReadSides(const SectorImage&,std::stop_token);
}
