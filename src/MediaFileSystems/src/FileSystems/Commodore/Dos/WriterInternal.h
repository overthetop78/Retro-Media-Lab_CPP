#pragma once
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include "Constants.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Writing {
struct Geometry {
    SectorGeometry geometry;
    std::int32_t count;
    std::int32_t logical(std::int32_t track,std::int32_t sector) const;
    std::pair<std::int32_t,std::int32_t> address(std::int32_t block) const;
    SectorAddress physical(std::int32_t block) const;
    bool is1581() const noexcept;
    std::int32_t center() const noexcept;
};
Geometry resolve(std::string_view);
std::vector<std::uint8_t> nameBytes(std::string_view);
class Builder {
public:
    Builder(const MigrationPlan&,Geometry,const WritePolicy&,std::stop_token);
    SectorWritePlan build();
private:
    const MigrationPlan& plan_;Geometry geometry_;const WritePolicy& policy_;std::stop_token token_;
    std::vector<std::vector<std::uint8_t>> sectors_;std::vector<bool> free_;std::vector<std::int32_t> directories_;
    void validate() const;
    void reserveSystem();void allocateDirectories();
    std::vector<std::int32_t> allocate(std::size_t);
    std::optional<std::int32_t> tryAllocate(std::int32_t track,std::int32_t sector);
    void writeFiles();void writeFile(const MigrationEntry&,std::size_t);
    void dataChain(Bytes,const std::vector<std::int32_t>&);
    std::vector<std::int32_t> relativeSides(const std::string&,const std::vector<std::int32_t>&,std::uint8_t);
    void system();void system1541();void system1581();
    void bam1541(MutableBytes,std::int32_t);void bam1581(MutableBytes,std::int32_t);
    void address(MutableBytes,std::size_t,std::int32_t) const;
};
}
