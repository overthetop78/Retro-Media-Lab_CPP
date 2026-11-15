#include "SectorImageEncoder.h"
#include "../Codecs/Iso/FmConstants.h"
#include "../Codecs/Iso/MfmConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <map>

namespace RetroMediaLab::MediaEngine::Encoding {
namespace {
namespace P = Reconstruction::Sector;
using Image = MediaFileSystems::SectorImage;
using Key = std::pair<std::int64_t,std::int64_t>;
using Groups = std::map<Key,std::vector<P::Item>>;
void visitBlocks(const Image& image, const std::function<bool(std::uint64_t)>& visitor, std::stop_token token) {
    try { image.visitAvailableBlocks(visitor, token); }
    catch (const MediaFileSystems::MediaError& error) {
        if (error.diagnostic().code == MediaFileSystems::DiagnosticCode::Cancelled)
            throw MediaError(DiagnosticCode::Cancelled);
        throw;
    }
}
std::shared_ptr<const P::Policy> requirePolicy(const P::Registry& policies, const Image& image) {
    if (auto policy = policies.resolve(image)) return policy;
    throw MediaError(DiagnosticCode::ImageEncodingPolicy, {image.geometry().formatId});
}
void physicalAddressing(const Image& image) {
    if (image.geometry().addressing == MediaFileSystems::SectorAddressingKind::Logical)
        throw MediaError(DiagnosticCode::ImageEncodingLogical, {image.geometry().formatId});
}
void order(std::vector<P::Item>& items) {
    const auto rank = [](const P::Item& item) {
        return item.block->recordedTrackOrder ? std::int64_t(*item.block->recordedTrackOrder) : item.address.number;
    };
    std::stable_sort(items.begin(), items.end(), [&](const auto& left, const auto& right) { return rank(left) < rank(right); });
}
std::vector<P::Item> collectTrack(const Image& image, const P::Policy& policy, Key track, std::stop_token token) {
    std::vector<P::Item> items;
    visitBlocks(image, [&](auto index) {
        checkCancellation(token);
        auto block = P::availableBlock(image, index, token);
        const auto address = policy.visualAddress(image, block->address);
        if (address.cylinder == track.first && address.head == track.second) items.push_back({std::move(block), address});
        return true;
    }, token);
    return items;
}
Attributes physicalAttributes(std::string_view encoderId, const Attributes& supplied, bool complete) {
    if (encoderId != IsoFm::Constants::Id && encoderId != IsoMfm::Constants::Id) return supplied;
    auto attributes = supplied.value_or(std::map<std::string,std::int32_t>{});
    if (encoderId == IsoFm::Constants::Id)
        attributes[std::string(IsoFm::Constants::PhysicalAttribute)] = IsoFm::Constants::PhysicalEnabled;
    else attributes[std::string(IsoMfm::Constants::PhysicalAttribute)] = IsoMfm::Constants::PhysicalEnabled;
    if (complete) attributes[std::string(IsoFm::Constants::CompleteAttribute)] = Constants::RequireCompletePhysicalTrack;
    return attributes;
}
}
SectorImageEncoder::SectorImageEncoder() = default;
SectorImageEncoder::SectorImageEncoder(Registry encoders, P::Registry policies)
    : encoders_(std::move(encoders)), policies_(std::move(policies)) {}
bool SectorImageEncoder::canEncode(const Image& image) const {
    return image.geometry().addressing == MediaFileSystems::SectorAddressingKind::Physical && bool(policies_.resolve(image));
}
EncodedTrack SectorImageEncoder::encodeTrack(const Image& image, std::int32_t cylinder, std::int32_t head,
    std::string_view encoderId, const Attributes& attributes, std::uint32_t cellTicks,
    std::uint32_t indexTicks, std::stop_token token) const {
    checkCancellation(token);
    physicalAddressing(image);
    const auto policy = requirePolicy(policies_, image);
    auto items = collectTrack(image, *policy, {cylinder, head}, token);
    order(items);
    auto sectors = policy->createTrackSectors(image, items, token);
    return encoders_.encode(encoderId, {cylinder, head, std::move(sectors), attributes, cellTicks, indexTicks}, token);
}
EncodedTrack SectorImageEncoder::encodePhysicalTrack(const Image& image, std::int32_t cylinder,
    std::int32_t head, std::uint32_t indexNanoseconds, std::stop_token token) const {
    checkCancellation(token);
    const auto policy = requirePolicy(policies_, image);
    const auto id = policy->encoderId(image);
    const auto items = collectTrack(image, *policy, {cylinder, head}, token);
    const auto attributes = physicalAttributes(id, policy->trackAttributes(image, P::checkedNumber(std::int64_t(items.size()))), false);
    const auto cellTicks = policy->bitCellTicks(image, cylinder);
    if (cellTicks > std::numeric_limits<std::uint32_t>::max() / Constants::TickNanoseconds)
        throw MediaError(DiagnosticCode::ImageEncodingDuration, {std::to_string(cellTicks), std::to_string(Constants::TickNanoseconds)});
    return encodeTrack(image, cylinder, head, id, attributes, cellTicks * Constants::TickNanoseconds, indexNanoseconds, token);
}
std::vector<DiskTrack> SectorImageEncoder::encode(const Image& image, std::stop_token token) const { return encodeImage(image, false, token); }
std::vector<DiskTrack> SectorImageEncoder::encodePhysical(const Image& image, std::stop_token token) const { return encodeImage(image, true, token); }
std::vector<DiskTrack> SectorImageEncoder::encodeImage(const Image& image, bool physical, std::stop_token token) const {
    physicalAddressing(image);
    const auto policy = requirePolicy(policies_, image);
    Groups groups;
    visitBlocks(image, [&](auto index) {
        checkCancellation(token);
        auto block = P::availableBlock(image, index, token);
        const auto address = policy->visualAddress(image, block->address);
        groups[{address.cylinder,address.head}].push_back({std::move(block), address});
        return true;
    }, token);
    std::vector<DiskTrack> tracks;
    for (auto& [key,items] : groups) {
        checkCancellation(token);
        order(items);
        auto sectors = policy->createTrackSectors(image, items, token);
        if (sectors.empty()) continue;
        const auto cylinder = P::checkedNumber(key.first), head = P::checkedNumber(key.second);
        const auto cellTicks = policy->bitCellTicks(image, cylinder), indexTicks = policy->indexTimeTicks(image, cylinder);
        const auto id = policy->encoderId(image);
        auto attributes = policy->trackAttributes(image, P::checkedNumber(std::int64_t(sectors.size())));
        if (physical) attributes = physicalAttributes(id, attributes, true);
        try {
            auto encoded = encoders_.encode(id, {cylinder, head, std::move(sectors), std::move(attributes), cellTicks, indexTicks}, token);
            tracks.push_back({cylinder, head, cellTicks, std::move(encoded)});
        } catch (const MediaError& error) {
            if (error.diagnostic().code == DiagnosticCode::EncoderRegistryUnknown)
                throw MediaError(DiagnosticCode::ImageEncodingPolicy, {image.geometry().formatId});
            throw;
        }
    }
    if (tracks.empty()) throw MediaError(DiagnosticCode::ImageEncodingNoTrack, {image.geometry().formatId});
    return tracks;
}
}

