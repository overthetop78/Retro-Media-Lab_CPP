#include "Automatic.h"
#include "RuntimeConstants.h"
#include "../../Formats/FormatIds.h"
#include "../../Formats/Epson/Geometry.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/Adf/Constants.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Raw/Constants.h"
#include "../../Formats/BbcDfs/Constants.h"
#include "../../Formats/Atari8Bit/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace M=MediaFileSystems;
namespace {
std::optional<std::string> epsonFormat(const Candidates& candidates,std::stop_token token){
    std::vector<Epson::SectorDescriptor> descriptors;
    for(const auto& group:candidates.entries()){
        checkCancellation(token);int priority=-1;std::map<std::size_t,std::size_t> frequencies;
        for(const auto& candidate:group.candidates){
            if(!candidate.sector.data)continue;
            const auto integrity=candidate.sector.integrityValid;
            const int score=integrity?(*integrity?RuntimeConstants::Valid:RuntimeConstants::Invalid):RuntimeConstants::Unknown;
            if(score>priority){priority=score;frequencies.clear();}
            if(score==priority)++frequencies[candidate.sector.data->size()];
        }
        if(frequencies.empty())continue;
        const auto size=std::max_element(frequencies.begin(),frequencies.end(),[](auto a,auto b){return a.second==b.second?a.first<b.first:a.second<b.second;})->first;
        descriptors.push_back({static_cast<std::int32_t>(group.address.cylinder),static_cast<std::int32_t>(group.address.head),static_cast<std::int32_t>(group.address.number),static_cast<std::int32_t>(size)});
    }
    return Epson::detect(descriptors);
}
std::shared_ptr<const M::SectorImage> acornImage(const Candidates& candidates,std::stop_token token){
    if(candidates.empty())return {};
    const auto measured=measure(candidates,token);
    if(!measured.zeroBased||(measured.sectorSize!=Adf::Constants::BbcBlockSize&&measured.sectorSize!=Adf::Constants::ArchimedesBlockSize))return {};
    for(const auto& profile:Adf::acornGeometries()){
        checkCancellation(token);const auto& g=profile.geometry;
        if(!Primitives::asciiIdentifierStartsWithIgnoreCase(g.formatId,Formats::AcornAdfsPrefix)||g.blockSize!=static_cast<std::uint32_t>(measured.sectorSize)||g.heads!=static_cast<std::uint32_t>(measured.heads)||g.sectorsPerTrack!=static_cast<std::uint32_t>(measured.sectorsPerTrack))continue;
        UniformRequest request;request.formatId=g.formatId;request.sectorSize=static_cast<std::int32_t>(g.blockSize);request.cylinders=static_cast<std::int32_t>(g.cylinders);request.heads=static_cast<std::int32_t>(g.heads);request.sectorsPerTrack=static_cast<std::int32_t>(g.sectorsPerTrack);request.sectorIndex=[](auto address){return static_cast<std::int32_t>(address.number);};
        auto image=createUniform(candidates,request,token);
        if(g.blockSize==Adf::Constants::BbcBlockSize){
            const auto first=image->block(0,token),second=image->block(1,token);
            if(!first||!second||first->data.size()!=g.blockSize||second->data.size()!=g.blockSize)continue;
            auto bytes=first->data;bytes.insert(bytes.end(),second->data.begin(),second->data.end());
            if(M::FileCore::oldMapDeclaredCapacity(bytes)!=image->capacity())continue;
        }
        if(M::Adfs::canRead(*image,token))return image;
    }
    return {};
}
bool ibmDos(const Candidates& candidates,std::stop_token token){
    const auto boot=bestData(candidates,{0,0,1},token),fat=bestData(candidates,{0,0,RuntimeConstants::FatFirstSector},token);
    const auto descriptor=fat.empty()?std::uint8_t{}:fat.front();
    const bool legacy=!M::Fat::detectBpbGeometry(boot)&&Raw::ibmFromDescriptor(descriptor).has_value();
    std::string oem;
    if(boot.size()>=Raw::Constants::OemOffset+Raw::Constants::OemLength){
        oem.assign(boot.begin()+Raw::Constants::OemOffset,boot.begin()+Raw::Constants::OemOffset+Raw::Constants::OemLength);
        const auto padding=[](char value){return value==RuntimeConstants::Null||value==RuntimeConstants::Space;};
        while(!oem.empty()&&padding(oem.back()))oem.pop_back();
        oem.erase(oem.begin(),std::find_if_not(oem.begin(),oem.end(),padding));
    }
    const bool dos=std::any_of(RuntimeConstants::DosOems.begin(),RuntimeConstants::DosOems.end(),[&](auto prefix){return Primitives::asciiIdentifierStartsWithIgnoreCase(oem,prefix);});
    return (legacy||dos)&&Raw::ibmFromBoot(boot,descriptor).has_value();
}
}
std::shared_ptr<const M::SectorImage> buildAutomatic(const CandidateSet& set,std::stop_token token){
    if(const auto format=epsonFormat(set.physical,token))return buildEpson(*format,set.physical,token);
    const auto measured=measure(set.addressed,token);
    if(auto image=acornImage(set.physical,token))return image;
    if(measured.zeroBased&&measured.sectorSize==BbcDfs::Constants::SectorSize&&measured.sectorsPerTrack==BbcDfs::Constants::SectorsPerTrack)return buildBbc(set,token);
    if(measured.sectorSize==Raw::Constants::SectorSize&&!measured.zeroBased&&ibmDos(set.addressed,token))return buildIbmAutomatic(set,token);
    namespace A=Atari8Bit::Constants;
    const bool atari=(measured.sectorSize==A::SingleSize||measured.sectorSize==A::DoubleSize)&&measured.heads==A::Heads&&(measured.sectorsPerTrack==A::StandardSectors||measured.sectorsPerTrack==A::EnhancedSectors);
    return atari?buildAtari8Bit(set,{},token):buildSt(set,{},token);
}
}
