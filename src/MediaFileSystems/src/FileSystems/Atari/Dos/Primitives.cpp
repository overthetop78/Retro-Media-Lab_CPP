#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::AtariDos {
std::int32_t extendedVtocSectorCount(std::uint8_t code,std::int32_t size) noexcept{using namespace Constants;return code<MinimumExtendedVtocCode?0:size==MinimumSectorSize?(code-ExtendedCodeBias)*ExtendedSingleDensityMultiplier:code-ExtendedCodeBias;}
bool vtocLooksValid(Bytes data,std::int32_t maximum) noexcept{
    using namespace Constants;if(data.size()<MinimumSectorSize)return false;const auto code=data[0];const bool standard=code==LegacyVtocMarker||code==VtocMarker||code==DoubleDensityVtocMarker,extended=code>=MinimumExtendedVtocCode;if(!standard&&!extended)return false;
    const auto usable=Binary::little<std::uint16_t>(data,UsableSectorCountOffset),free=Binary::little<std::uint16_t>(data,FreeSectorCountOffset);if(usable==0||usable>maximum||free>usable)return false;if(!extended)return true;
    const auto count=extendedVtocSectorCount(code,std::int32_t(data.size()));return count>0&&count<VtocSector&&usable==std::int64_t(maximum)-BootSectorCount-DirectorySectorCount-count;
}
bool trySector(const SectorImage& image,std::int32_t number,std::vector<std::uint8_t>& data,std::stop_token token){checkCancellation(token);data.clear();if(number<=0)return false;const auto block=image.block(std::uint64_t(number-1),token);if(!block)return false;data=block->data;return true;}
std::optional<std::int32_t> freeSectors(const SectorImage& image,std::stop_token token){std::vector<std::uint8_t> data;if(!trySector(image,Constants::VtocSector,data,token)||data.size()<Constants::FreeSectorCountOffset+Constants::FreeSectorCountLength)return {};return Binary::little<std::uint16_t>(data,Constants::FreeSectorCountOffset);}
std::string decodeName(Bytes raw){
    if(raw.size()<Constants::NameLength)throw MediaError(DiagnosticCode::ReadOutOfBounds);const auto decode=[](Bytes bytes){QByteArray clean;clean.reserve(qsizetype(bytes.size()));for(auto value:bytes)clean.push_back(char(value<=Constants::AsciiMaximum?value:Constants::AsciiReplacement));return QString::fromLatin1(clean).trimmed().toUtf8().toStdString();};auto name=decode(raw.first(Constants::NameLength));auto extension=decode(raw.subspan(Constants::NameLength));return extension.empty()?name:name+Constants::Dot+extension;
}
bool directoryLooksValid(Bytes data){
    if(data.size()<Constants::MinimumSectorSize)return false;for(std::int32_t slot=0;slot<Constants::DirectoryEntriesPerSector;++slot){const auto offset=std::size_t(slot)*Constants::DirectoryEntrySize;const auto flags=data[offset];if(flags==0)return true;if(!Internal::validEntry(data,offset,flags,true))return false;}return true;
}
namespace Internal {
bool extended(const SectorImage& image,std::stop_token token){std::vector<std::uint8_t> data;return trySector(image,Constants::VtocSector,data,token)&&!data.empty()&&data[0]>=Constants::MinimumExtendedVtocCode;}
DirectoryLocation canonical() noexcept{return {Constants::FirstDirectorySector,Constants::DirectorySectorCount,Constants::DirectoryEntriesPerSector};}
}
}
