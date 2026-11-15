#pragma once
#include <cstdint>
#include <string_view>
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive::Constants {
inline constexpr std::uint32_t DirectoryStartBlock=2;
inline constexpr std::size_t NameLength=12,EntryLength=16,SizeOffset=12;
inline constexpr std::uint8_t Padding=255,AsciiFirst=0x20,AsciiLast=0x7e;
inline constexpr std::uint64_t MaximumRangeLength=2147483647;
inline constexpr std::string_view ReservedName="Reserved",FileSystemId="amiga-flat-resource-archive";
inline constexpr std::string_view EntryComment="Flat resource archive entry",CommentKey="Media.AmigaFlatEntryComment",TranslationDomain="MediaFileSystems";
inline constexpr std::string_view BlockSeparator=", ";
}
