#include "DefinedReader.h"
#include "Iso/Builder.h"
#include "../Formats/Definitions/Constants.h"
#include "../Formats/Definitions/RawImages.h"
#include "../Formats/Scp/ProtectedImage.h"
#include "../Sources/RandomAccessSources.h"
#include "../Codecs/Micropolis/Mfm.h"
#include "../Codecs/Northstar/Mfm.h"
#include "../Codecs/Iso/FmConstants.h"
#include "../Codecs/Iso/MfmConstants.h"
#include "../Codecs/AppleII/Constants.h"
#include "../Codecs/DataGeneral/Constants.h"
#include "../Formats/FormatIds.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::DefinedReconstruction {
namespace M=MediaFileSystems;namespace I=IsoReconstruction;namespace D=Definitions::Constants;
namespace {
Decoding::DecodeOperation completeRecords(const Decoding::Registry& registry){
    auto decoders=registry.decoders();
    for(auto& decoder:decoders)if(dynamic_cast<const Micropolis::Decoder*>(decoder.get()))decoder=std::make_shared<Micropolis::Decoder>(true);
    return Decoding::Registry(std::move(decoders)).operation();
}
std::string_view codecId(std::string_view codec){
    if(codec==D::IbmFm)return IsoFm::Constants::Id;if(codec==D::IbmMfm)return IsoMfm::Constants::Id;
    if(codec==D::AppleGcr)return AppleII::Constants::Id;if(codec==D::DataGeneral)return DataGeneral::Constants::Id;
    if(codec==D::Hp)return Formats::HpMmfm;if(codec==D::Micropolis)return Micropolis::Constants::Id;
    throw MediaError(DiagnosticCode::DefinedScpCodec,{std::string(codec)});
}
const Definitions::DiskDefinition& definition(std::string_view id){const auto& all=Definitions::rawDefinitions();const auto found=all.find(id);if(found==all.end())throw MediaError(DiagnosticCode::DefinedRawFormat,{std::string(id)});return found->second;}
std::shared_ptr<const M::SectorImage> northstar(const Flux::ProtectedTrackImage& image,const Definitions::DiskDefinition& definition,std::span<const Definitions::Sector> layout,std::stop_token token){
    std::vector<M::SectorBlock> blocks;
    for(const auto& track:image.tracks()){
        checkCancellation(token);const auto* profile=definition.track(track.cylinder(),track.head());if(!profile)continue;
        const auto mode=profile->parameters.find(D::Mode);const bool fm=mode!=profile->parameters.end()&&mode->second==D::FmMode;
        I::Candidates candidates;std::int32_t revolution=0;
        for(const auto& value:track.revolutions()){
            ++revolution;for(auto sector:Northstar::decodeHardSectors(value.flux(),fm,track.cylinder(),track.head(),profile->number(D::Sectors,D::DefaultNorthstarSectors),token))candidates.add({track.cylinder(),track.head(),sector.number},{std::move(sector),revolution,{}});
        }
        for(const auto& sector:layout){
            checkCancellation(token);if(sector.cylinder!=track.cylinder()||sector.head!=track.head())continue;
            const M::SectorAddress address{sector.cylinder,sector.head,sector.number};const auto* values=candidates.find(address);if(!values)continue;std::vector<I::Candidate> matching;
            for(const auto& value:*values)if(value.sector.data&&value.sector.data->size()==std::size_t(sector.size))matching.push_back(value);
            if(matching.empty())continue;const auto& best=I::best(matching,token);M::SectorBlock block;block.logicalBlock=sector.logicalBlock;block.address=address;block.data=*best.sector.data;block.integrityValid=best.sector.integrityValid;block.revolution=best.revolution;blocks.push_back(std::move(block));
        }
    }
    return Definitions::createImage(definition,layout,std::move(blocks),token);
}
}
Reader::Reader(Decoding::Registry registry,I::ReadCapture read):readCapture_(std::move(read)),candidates_(completeRecords(registry)){
    if(!readCapture_)readCapture_=[](const auto& path,auto token){return Scp::read(*fileSource(path),token);};
}
std::shared_ptr<const M::SectorImage> Reader::read(const std::filesystem::path& path,std::string_view format,const I::CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);return read(Scp::protectedImage(readCapture_(path,token),token),format,progress,token);
}
std::shared_ptr<const M::SectorImage> Reader::read(const Flux::ProtectedTrackImage& image,std::string_view format,const I::CollectionProgressCallback& progress,std::stop_token token){return read(image,definition(format),progress,token);}
std::shared_ptr<const M::SectorImage> Reader::read(const Flux::ProtectedTrackImage& image,const Definitions::DiskDefinition& definition,const I::CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);const auto layout=definition.layout();
    if(std::all_of(definition.tracks.begin(),definition.tracks.end(),[](const auto& track){return track.codec==D::Northstar;}))return northstar(image,definition,layout,token);
    std::map<std::string,I::SharedCandidates> decoded;
    for(const auto& track:definition.tracks){
        checkCancellation(token);if(decoded.contains(track.codec))continue;const std::array ids{std::string(codecId(track.codec))};decoded.emplace(track.codec,candidates_.decode(image,ids,progress,token));
    }
    std::vector<M::SectorBlock> blocks;
    for(const auto& sector:layout){
        checkCancellation(token);const auto& profile=definition.tracks.at(sector.trackIndex);const M::SectorAddress address{sector.cylinder,sector.head,sector.number};
        const auto* values=decoded.at(profile.codec)->physical.find(address);if(!values)continue;const bool micropolis=profile.codec==D::Micropolis;std::vector<I::Candidate> matching;
        for(const auto& value:*values)if(value.sector.cylinder==sector.cylinder&&(micropolis||value.sector.head==profile.number(D::RecordedHead,sector.head))&&value.sector.data&&value.sector.data->size()==std::size_t(micropolis?Micropolis::Constants::RecordByteCount:sector.size))matching.push_back(value);
        if(matching.empty())continue;const auto& best=I::best(matching,token);auto data=*best.sector.data;
        if(micropolis&&sector.size==Micropolis::Constants::SectorSize)data=std::vector<std::uint8_t>(data.begin()+Micropolis::Constants::DataOffset,data.begin()+Micropolis::Constants::DataOffset+Micropolis::Constants::SectorSize);
        M::SectorBlock block;block.logicalBlock=sector.logicalBlock;block.address=address;block.data=std::move(data);block.integrityValid=best.sector.integrityValid;block.revolution=best.revolution;blocks.push_back(std::move(block));
    }
    return Definitions::createImage(definition,layout,std::move(blocks),token);
}
}
