#pragma once
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>

namespace RetroMediaLab::MediaEngine::Formats {
using namespace MediaFileSystems::Formats;
inline constexpr std::string_view AmigaPrefix = "amiga.", AtariStPrefix = "atarist.";
inline constexpr std::string_view AcornDfsPrefix = "acorn.dfs.", AcornAdfsPrefix = "acorn.adfs.";
inline constexpr std::string_view IbmPrefix = "ibm.", MsxPrefix = "msx.", AmstradPrefix = "amstrad.";
inline constexpr std::string_view UcsdPrefix = "ucsd.";
inline constexpr std::string_view HpMmfm = "hp.mmfm", DataGeneralFm = "datageneral.fm", MicropolisMfm = "micropolis.mfm";
inline constexpr std::string_view MembrainMfm = "membrain.mfm", Aed6200pMfm = "aed6200p.mfm", QdMo5Mfm = "qdmo5.mfm";
inline constexpr std::string_view CenturionMfm = "centurion.mfm", NorthstarMfm = "northstar.mfm", HeathkitFm = "heathkit.fm";
inline constexpr std::string_view MicralNFm = "micraln.fm", EmuFm = "emu.fm", TycomFm = "tycom.fm";
inline constexpr std::string_view Arburg = "arburg", Victor9kGcr = "victor9k.gcr";
inline constexpr std::string_view AppleIIAppleDosPrefix="apple2.appledos",AppleIINoFileSystemPrefix="apple2.nofs",AppleIIDosPrefix="apple2.dos";
inline constexpr std::string_view AppleIIPrefix = "apple2.", AppleIIIPrefix = "apple3.", AppleLisaPrefix = "applelisa.", AppleMacPrefix = "applemac.", MacPrefix = "mac.";
inline constexpr std::string_view AppleIIRwts18 = "apple2.rwts18";
inline constexpr std::string_view CommodorePrefix = "commodore.", Commodore900Prefix = "commodore900.";
inline constexpr std::string_view DecRx02 = "dec.rx02", Imd = "imd", Td0 = "td0";
}

namespace RetroMediaLab::MediaEngine::Formats {
inline constexpr std::string_view AppleLisaRaw = "applelisa.raw";
}

