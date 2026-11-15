#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/AppleRecognition.h>
namespace RetroMediaLab::MediaEngine::Apple {
using Bytes=MediaFileSystems::Bytes;
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeImage(std::string_view extension,Bytes,std::optional<std::string_view> requestedFormat={}) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeDos33(Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeProDos(Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<MediaFileSystems::Apple::MacintoshKind> probeMacintosh(Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeLisaOffice(Bytes) noexcept;
RETROMEDIALAB_MEDIAENGINE_API bool looksLikeSos(Bytes) noexcept;
}
