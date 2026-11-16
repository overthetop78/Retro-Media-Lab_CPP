#pragma once
#include "Container.h"
namespace RetroMediaLab::MediaEngine::Vhdx {
using Guid=Constants::Guid;
struct Region{std::int64_t offset=0;std::uint32_t length=0;};
struct Metadata{std::uint32_t blockSize=0,fileFlags=0;std::int64_t virtualSize=0;std::int32_t logicalSector=0,physicalSector=0;Guid page83{};};
struct Structures{Region bat;Metadata metadata;};
Guid readGuid(std::span<const std::uint8_t>);
Guid randomGuid();
std::string guidText(const Guid&);
bool validCrc(std::span<const std::uint8_t>);
void writeCrc(std::span<std::uint8_t>);
Structures readStructures(const RecognitionContext&,std::stop_token);
Metadata readMetadata(const RecognitionContext&,Region,std::stop_token);
}
