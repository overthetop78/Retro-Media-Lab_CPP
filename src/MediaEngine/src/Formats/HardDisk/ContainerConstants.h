#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::HardDiskContainers::Constants {
inline constexpr std::string_view VariantKey="variant",FixedVariant="Fixed",DynamicVariant="Dynamic";
inline constexpr int FixedEnumValue=1,DynamicEnumValue=2;
}
