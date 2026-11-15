#include "Internal.h"
#include <chrono>
#include "../../../Primitives/MetadataConstants.h"
namespace RetroMediaLab::MediaFileSystems::Coherent {
using namespace Constants;
std::uint32_t canonicalUInt32(Bytes value){
 if(value.size()<CanonicalLength)throw MediaError(DiagnosticCode::CoherentCanonicalLength,{std::to_string(value.size()),std::to_string(CanonicalLength)});
 return std::uint32_t(value[CanonicalLow])|(std::uint32_t(value[CanonicalLowMiddle])<<BitsPerByte)|(std::uint32_t(value[CanonicalHighMiddle])<<HighMiddleShift)|(std::uint32_t(value[CanonicalHigh])<<HighShift);
}
std::uint32_t blockPointer(Bytes value){if(value.size()<PointerSize)throw MediaError(DiagnosticCode::CoherentPointerLength,{std::to_string(value.size()),std::to_string(PointerSize)});return std::uint32_t(value[PointerLow])|(std::uint32_t(value[PointerMiddle])<<BitsPerByte)|(std::uint32_t(value[PointerHigh])<<HighMiddleShift);}
std::string decodeName(Bytes bytes){std::string value;value.reserve(bytes.size());for(const auto c:bytes)value+=c<AsciiLimit?char(c):Replacement;while(!value.empty()&&(value.back()==Zero||value.back()==Space||value.back()==LineFeed||value.back()==Return))value.pop_back();return value;}
std::string inodeDescription(std::uint16_t number){return std::string(InodePrefix)+std::to_string(number);}
EntryKind entryKind(std::uint16_t mode) noexcept {const auto type=mode&TypeMask;return type==DirectoryMode?EntryKind::Directory:type==RegularMode?EntryKind::File:EntryKind::Unknown;}
std::optional<Timestamp> decodeTime(std::uint32_t seconds){if(!seconds)return {};using namespace std::chrono;const sys_seconds instant{std::chrono::seconds(seconds)};const auto date=year_month_day(floor<days>(instant));const hh_mm_ss clock(instant-floor<days>(instant));return Timestamp{int(date.year()),int(unsigned(date.month())),int(unsigned(date.day())),int(clock.hours().count()),int(clock.minutes().count()),int(clock.seconds().count()),0,0};}
bool looksLike(Bytes bytes){
 if(bytes.size()<MinimumImageSize)return false;
 const auto name=bytes.subspan(VolumeNameOffset,NameLength),pack=bytes.subspan(PackNameOffset,NameLength);
 const auto ascii=[](Bytes b){std::string s;for(const auto c:b)s+=c<AsciiLimit?char(c):Replacement;return s;};
 const auto n=ascii(name),p=ascii(pack);return (n==DefaultName||n==std::string(Placeholder)+Space)&&(p==DefaultPack||p==std::string(Placeholder)+LineFeed);
}
std::uint32_t declaredBlocks(Bytes bytes){if(bytes.size()<BlockCountOffset+CanonicalLength)throw MediaError(DiagnosticCode::CoherentInvalidSuperblock);const auto value=canonicalUInt32(bytes.subspan(BlockCountOffset,CanonicalLength));if(value>MaximumFileSize)throw MediaError(DiagnosticCode::CoherentDeclaredBlocks,{std::to_string(value),std::to_string(MaximumFileSize)});return value;}
std::uint32_t validatedBlocks(Bytes bytes,std::uint64_t length){
 if(length%BlockSize||!looksLike(bytes))throw MediaError(DiagnosticCode::CoherentInvalidSuperblock);
 const auto count=declaredBlocks(bytes);if(count<MinimumInodeZoneEnd||count>length/BlockSize)throw MediaError(DiagnosticCode::CoherentDeclaredBlocks,{std::to_string(count),std::to_string(length/BlockSize)});return count;
}
bool canRead(const SectorImage& image){const auto& format=image.geometry().formatId;if(format.size()!=FormatId.size()||image.geometry().blockSize!=BlockSize)return false;for(std::size_t i=0;i<format.size();++i){const auto c=format[i];if((c>=MetadataConstants::UpperFirst&&c<=MetadataConstants::UpperLast?char(c+MetadataConstants::CaseDelta):c)!=FormatId[i])return false;}return true;}
}
