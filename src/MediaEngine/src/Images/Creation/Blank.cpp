#include "Blank.h"
#include "Constants.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/St/Geometry.h"
#include "../../Formats/St/Constants.h"
#include "../../Formats/Apple/Geometry.h"
#include "../../Formats/AcornAtom/Constants.h"
#include "../../Formats/BbcDfs/Constants.h"
#include "../../Formats/CommodoreDisks/Images.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
namespace M = MediaFileSystems;
std::shared_ptr<const M::SectorImage> linear(M::SectorGeometry geometry, std::int64_t first = 0) {
    const auto size=std::uint64_t(geometry.blockSize)*geometry.cylinders*geometry.heads*geometry.sectorsPerTrack;
    SourceSectorMetadata metadata;metadata.firstSectorNumber=first;metadata.trustCompletePayload=true;
    return sourceSectorImage(memorySource(std::vector<std::uint8_t>(std::size_t(size))),std::move(geometry),0,{},metadata);
}
std::shared_ptr<const M::SectorImage> catalogueImage(M::SectorGeometry geometry,bool atom) {
    using namespace CreationConstants;
    const auto trackSize=geometry.blockSize*geometry.sectorsPerTrack;
    std::vector<std::uint8_t> bytes(std::size_t(geometry.cylinders)*geometry.heads*trackSize);
    const auto sectors=geometry.cylinders*geometry.sectorsPerTrack;
    for(std::uint32_t head=0;head<geometry.heads;++head){const auto offset=head*trackSize+geometry.blockSize;bytes[offset+CatalogueHigh]=std::uint8_t(sectors>>ByteShift);bytes[offset+CatalogueLow]=std::uint8_t(sectors);}
    SourceSectorMetadata metadata;metadata.firstSectorNumber=0;metadata.trustCompletePayload=true;
    if(!atom)metadata.recordedHead=0;
    return sourceSectorImage(memorySource(bytes),std::move(geometry),0,{},metadata);
}
}
std::shared_ptr<const MediaFileSystems::SectorImage> BlankSectorImageFactory::create(std::string_view id,std::stop_token token) {
    checkCancellation(token);
    using namespace Formats;
    if(id==AcornDosPlus800||id==AcornAdfs800){
        const auto profile=*Adf::findGeometry(id);std::vector<std::uint8_t> bytes(Adf::Constants::Capacity);
        if(id==AcornDosPlus800){bytes[0]=Adf::Constants::DosDescriptor;bytes[1]=bytes[Adf::Constants::FatHeaderLastByte]=Adf::Constants::FatHeaderByte;}
        else {const auto header=MediaFileSystems::Adfs::createDHeader();std::copy(header.begin(),header.end(),bytes.begin());}
        SourceSectorMetadata metadata;metadata.firstSectorNumber=profile.firstSectorNumber;metadata.trustCompletePayload=true;
        return sourceSectorImage(memorySource(bytes),profile.geometry,0,{},metadata);
    }
    if(id==AcornAtomDos){using namespace AcornAtom::Constants;return catalogueImage({std::string(id),BlockSize,Cylinders,Heads,SectorsPerTrack},true);}
    for(const auto& profile:Adf::acornGeometries())if(profile.geometry.formatId==id&&profile.geometry.blockSize==Adf::Constants::BbcBlockSize){
        const auto& g=profile.geometry;const auto count=g.cylinders*g.heads*g.sectorsPerTrack;
        std::vector<std::uint8_t> bytes(std::size_t(count)*g.blockSize);const auto header=MediaFileSystems::Adfs::createBbcHeader(count);std::copy(header.begin(),header.end(),bytes.begin());
        SourceSectorMetadata metadata;metadata.firstSectorNumber=profile.firstSectorNumber;metadata.recordedHead=0;metadata.trustCompletePayload=true;
        return sourceSectorImage(memorySource(bytes),g,0,{},metadata);
    }
    if(const auto g=BbcDfs::findGeometry(id))return catalogueImage(*g,false);
    if(const auto g=St::geometry(id))return linear(*g,St::Constants::FirstSector);
    for(const auto& g:Raw::ibmGeometries())if(g.formatId==id)return linear(g,1);
    for(const auto& g:Raw::msxGeometries())if(g.formatId==id)return linear(g,1);
    for(const auto& profile:Adf::amigaGeometries())if(profile.geometry.formatId==id)return linear(profile.geometry,profile.firstSectorNumber);
    using namespace Apple::Geometry;
    if(id==AppleIIAppleDos113||id==AppleIIAppleDos140)return linear({std::string(id),AppleII::SectorSize,AppleII::TrackCount,SingleHead,std::uint32_t(id==AppleIIAppleDos113?AppleII::Dos32SectorsPerTrack:AppleII::SectorsPerTrack)});
    if(id==AppleIIProDos140||id==AppleIIISos)return linear({std::string(id),AppleII::ProDosBlockSize,AppleII::TrackCount,SingleHead,AppleII::ProDosBlocksPerTrack});
    if(id==AppleIIProDos800){
        const auto g=MacintoshGcr::forHeads(MacintoshGcr::DoubleSidedHeadCount);SourceSectorMetadata metadata;metadata.trustCompletePayload=true;
        metadata.addressOf=[](std::uint64_t logical){return MacintoshGcr::address(std::int32_t(logical),MacintoshGcr::DoubleSidedHeadCount);};
        return sourceSectorImage(memorySource(std::vector<std::uint8_t>(g.capacity)),{std::string(id),MacintoshGcr::BlockSize,MacintoshGcr::CylinderCount,MacintoshGcr::DoubleSidedHeadCount,MacintoshGcr::MaximumSectorsPerTrack},0,{false,std::uint64_t(g.capacity),std::uint64_t(g.blockCount)},metadata);
    }
    for(const auto kind:{CommodoreDisks::Kind::D64,CommodoreDisks::Kind::D71,CommodoreDisks::Kind::D81})if(id==CommodoreDisks::formatId(kind))
        return CommodoreDisks::read(kind,memorySource(std::vector<std::uint8_t>(std::size_t(CommodoreDisks::layouts(kind).front().imageLength))),token);
    throw MediaError(DiagnosticCode::MigrationTarget,{std::string(id)});
}
}
