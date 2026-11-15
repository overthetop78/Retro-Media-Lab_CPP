#pragma once
#include "Policy.h"

namespace RetroMediaLab::MediaEngine::Reconstruction::Sector {
class ApplePolicy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API Address visualAddress(const Image&, Address) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Encoding::Sector> createTrackSectors(const Image&, std::span<const Item>, std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAENGINE_API Encoding::Attributes trackAttributes(const Image&, std::int32_t) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t bitCellTicks(const Image&, std::int32_t) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t indexTimeTicks(const Image&, std::int32_t) const override;
};
class CommodorePolicy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API Address visualAddress(const Image&, Address) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Encoding::Sector> createTrackSectors(const Image&, std::span<const Item>, std::stop_token = {}) const override;
    RETROMEDIALAB_MEDIAENGINE_API Encoding::Attributes trackAttributes(const Image&, std::int32_t) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t bitCellTicks(const Image&, std::int32_t) const override;
};
class Rx02Policy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Encoding::Sector> createTrackSectors(const Image&, std::span<const Item>, std::stop_token = {}) const override;
};
class AtariPolicy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API Address visualAddress(const Image&, Address) const override;
};
class RarePolicy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t bitCellTicks(const Image&, std::int32_t) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t indexTimeTicks(const Image&, std::int32_t) const override;
};
class JfdPolicy final : public Policy {
public:
    RETROMEDIALAB_MEDIAENGINE_API bool canHandle(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::string_view encoderId(const Image&) const override;
    RETROMEDIALAB_MEDIAENGINE_API std::uint32_t bitCellTicks(const Image&, std::int32_t) const override;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Policy> prefixPolicy(std::string encoderId, std::vector<std::string> prefixes);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Policy> exactPolicy(std::string encoderId, std::vector<std::string> formats);
}

