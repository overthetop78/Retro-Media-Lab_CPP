#pragma once
#include "../../Models/Flux/ProtectedTrack.h"
#include <map>
#include <string>
namespace RetroMediaLab::MediaEngine::HxcStream {
using Metadata=std::map<std::string,std::string>;
RETROMEDIALAB_MEDIAENGINE_API std::string nativeKey(std::int32_t cylinder,std::int32_t head);
RETROMEDIALAB_MEDIAENGINE_API std::string digestKey(std::int32_t cylinder,std::int32_t head);
RETROMEDIALAB_MEDIAENGINE_API std::string fieldsPrefix(std::int32_t cylinder,std::int32_t head);
RETROMEDIALAB_MEDIAENGINE_API std::string digest(const Flux::ProtectedTrack&,bool writeProtected);
RETROMEDIALAB_MEDIAENGINE_API bool canEncode(const Flux::ProtectedTrack&);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encode(const Flux::ProtectedTrack&,bool writeProtected,const Metadata& = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::uint32_t checksum(std::span<const std::uint8_t>,std::uint32_t seed);
}
