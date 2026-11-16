#include "Reader.h"
#include "Constants.h"
#include "../../Formats/Apple/Probe.h"
#include "../../Formats/FormatIds.h"
#include "../../Models/Sectors/Builders.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Images/ReadRejection.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AppleScp {
Reader::Reader(Decoding::Registry registry,IsoReconstruction::ReadCapture read,MacDecode mac):decoder_(registry.operation(),std::move(mac)),readCapture_(std::move(read)){
    if(!readCapture_)readCapture_=[](const auto& path,auto token){return Scp::read(*fileSource(path),token);};
}
std::shared_ptr<const MediaFileSystems::SectorImage> Reader::read(const std::filesystem::path& path,std::optional<std::string> format,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token)const{
    checkCancellation(token);return read(readCapture_(path,token),std::move(format),progress,token);
}
std::shared_ptr<const MediaFileSystems::SectorImage> Reader::read(const Scp::Image& image,std::optional<std::string> format,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token)const{
    checkCancellation(token);const auto prefix=[&](std::string_view value){return format&&Primitives::asciiIdentifierStartsWithIgnoreCase(*format,value);};
    if(prefix(Formats::AppleIIRwts18))return rwts18(image,decoder_,progress,token);
    if(prefix(Formats::AppleIIAppleDosPrefix)||prefix(Formats::AppleIINoFileSystemPrefix)||prefix(Formats::AppleIIDosPrefix))return appleII(image,decoder_,false,progress,token);
    if(prefix(Formats::AppleIIISos)){
        const auto provisional=appleII(image,decoder_,true,progress,token);std::vector<MediaFileSystems::SectorBlock> blocks;
        for(std::uint64_t logical=0;logical<provisional->blockCount();++logical)blocks.push_back(*MediaFileSystems::requireSectorBlock(*provisional,logical,token));
        const auto payload=denseSectorImage(blocks,static_cast<std::int32_t>(provisional->blockCount()),static_cast<std::int32_t>(provisional->geometry().blockSize),token);
        if(!Apple::looksLikeSos(payload.data))throw MediaError(DiagnosticCode::AppleScpSos);return relabelSectorImage(provisional,std::string(Formats::AppleIIISos));
    }
    if(prefix(Formats::AppleIIProDos140))return appleII(image,decoder_,true,progress,token);
    if(prefix(Formats::AppleIIProDos800)||prefix(Formats::MacPrefix)||prefix(Formats::AppleMacPrefix)||prefix(Formats::AppleLisaPrefix)||(format&&Primitives::asciiIdentifierEqualsIgnoreCase(*format,Formats::AppleIIProDos)))return macintosh(image,decoder_,format,progress,token);
    std::shared_ptr<const MediaFileSystems::SectorImage> selected;double bestRatio=-1;std::uint64_t bestCount=0;std::vector<DiagnosticCause> failures;
    const auto attempt=[&](std::string_view identity,const auto& decode){
        try{
            auto candidate=decode();std::uint64_t available=0;candidate->visitAvailableBlocks([&](auto){++available;return true;},token);const auto ratio=static_cast<double>(available)/static_cast<double>(std::max(std::uint64_t(1),candidate->blockCount()));
            if(!selected||ratio>bestRatio||(ratio==bestRatio&&available>bestCount)){selected=std::move(candidate);bestRatio=ratio;bestCount=available;}
        }catch(...){const auto failure=std::current_exception();if(!isImageReadRejection(failure))std::rethrow_exception(failure);failures.push_back({std::string(identity),failure});}
    };
    attempt(Constants::MacintoshName,[&]{return macintosh(image,decoder_,{},progress,token);});
    attempt(Constants::AppleIIName,[&]{return appleII(image,decoder_,false,progress,token);});
    attempt(Constants::RwtsName,[&]{return rwts18(image,decoder_,progress,token);});
    if(!selected){std::string identities;for(const auto& failure:failures){if(!identities.empty())identities+=Constants::FailureSeparator;identities+=failure.identity;}throw MediaError(DiagnosticCode::AppleScpRejected,{std::move(identities)},std::move(failures));}
    return selected;
}
}
