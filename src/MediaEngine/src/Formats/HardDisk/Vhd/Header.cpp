#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QUuid>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhd {
std::uint32_t checksum(std::span<const std::uint8_t> data,std::size_t offset){std::uint32_t sum{};for(std::size_t i=0;i<data.size();++i)if(i<offset||i>=offset+Constants::EntryBytes)sum+=data[i];return ~sum;}
Footer readFooter(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);
    if(context.length()<std::int64_t(FooterSize))throw MediaError(DiagnosticCode::VhdShortFooter);
    const auto bytes=context.readAsync(context.length()-FooterSize,FooterSize,token).get();
    if(!std::equal(FooterCookie.begin(),FooterCookie.end(),bytes.begin()))throw MediaError(DiagnosticCode::VhdCookie);
    if(qFromBigEndian<std::uint32_t>(bytes.data()+ChecksumOffset)!=checksum(bytes,ChecksumOffset))throw MediaError(DiagnosticCode::VhdFooterChecksum);
    const auto version=qFromBigEndian<std::uint32_t>(bytes.data()+VersionOffset);if(version!=Version)throw MediaError(DiagnosticCode::VhdVersion,{std::to_string(version)});
    const auto size=qFromBigEndian<std::uint64_t>(bytes.data()+CurrentSizeOffset);if(!size||size>MaximumLogicalSize)throw MediaError(DiagnosticCode::VhdSize);if(size%SectorSize)throw MediaError(DiagnosticCode::VhdAlignment);
    const auto uuid=QUuid::fromRfc4122(QByteArray(reinterpret_cast<const char*>(bytes.data()+UuidOffset),GuidBytes));
    return {qFromBigEndian<std::uint64_t>(bytes.data()+DataOffset),std::int64_t(size),qFromBigEndian<std::uint16_t>(bytes.data()+CylindersOffset),bytes[HeadsOffset],bytes[SectorsOffset],qFromBigEndian<std::uint32_t>(bytes.data()+TypeOffset),uuid.toString(QUuid::WithoutBraces).toStdString()};
}
}
