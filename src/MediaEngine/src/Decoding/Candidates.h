#pragma once
#include <optional>
namespace RetroMediaLab::MediaEngine::Decoding {
// Strict comparison retains the first candidate on equal integrity, like the stable C# ordering.
inline bool betterIntegrity(std::optional<bool> candidate,std::optional<bool> current) noexcept {
    if(candidate==true)return current!=true;
    return !candidate.has_value()&&current==false;
}
}
