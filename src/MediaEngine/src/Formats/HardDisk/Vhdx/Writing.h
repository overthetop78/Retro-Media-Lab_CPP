#pragma once
#include "Structures.h"
namespace RetroMediaLab::MediaEngine::Vhdx {
std::vector<std::uint8_t> createIdentifier();
std::vector<std::uint8_t> createHeader(std::uint64_t sequence,const Guid& fileId,const Guid& dataId,std::uint64_t logOffset);
std::vector<std::uint8_t> createRegions(std::uint32_t batLength);
std::vector<std::uint8_t> createMetadata(const BlockImage&,std::uint32_t blockSize,bool fixed);
}
