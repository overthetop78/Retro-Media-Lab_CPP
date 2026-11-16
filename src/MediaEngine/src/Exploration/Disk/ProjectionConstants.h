#pragma once
#include <string_view>
namespace RetroMediaLab::MediaEngine::DiskProjection::Constants {
inline constexpr std::string_view Signature="SCP",Flags="flags",BitCellEncoding="bitCellEncoding",Heads="heads",Extension="extension",AmigaEncoding="amiga-mfm",IbmEncoding="ibm-mfm";
inline constexpr int HexBase=16,ChecksumWidth=8;
inline constexpr char ChecksumPadding='0',ExtensionDot='.';
}
