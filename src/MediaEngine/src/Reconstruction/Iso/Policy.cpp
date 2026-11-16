#include "Policy.h"
#include "RuntimeConstants.h"
#include "../../Formats/FormatIds.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Codecs/Iso/FmConstants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
Policy resolvePolicy(const std::optional<std::string>& format){
    Policy policy{PolicyKind::Automatic,{std::string(IsoFm::Constants::Id),std::string(IsoMfm::Constants::Id)}};
    if(!format)return policy;
    const auto same=Primitives::asciiIdentifierEqualsIgnoreCase,prefix=Primitives::asciiIdentifierStartsWithIgnoreCase;
    if(same(*format,Formats::AcornAtomDos))policy.kind=PolicyKind::Generic;
    else if(prefix(*format,Formats::AcornAdfsPrefix)&&Adf::findGeometry(*format)){policy.kind=PolicyKind::AcornAdfs;policy.decoderIds={std::string(IsoMfm::Constants::Id)};}
    else if(prefix(*format,Formats::AtariPrefix)){policy.kind=PolicyKind::Atari8Bit;policy.decoderIds={std::string(*format==Formats::Atari90?IsoFm::Constants::Id:IsoMfm::Constants::Id)};}
    else if(prefix(*format,Formats::AtariStPrefix)){policy.kind=PolicyKind::AtariSt;policy.decoderIds={std::string(IsoMfm::Constants::Id)};}
    else if(prefix(*format,Formats::AmstradPrefix))policy.kind=PolicyKind::Amstrad;
    else if(prefix(*format,Formats::IbmPrefix)||same(*format,Formats::Mac1440))policy.kind=PolicyKind::Ibm;
    else if(prefix(*format,Formats::AcornDfsPrefix)){policy.kind=PolicyKind::Bbc;policy.decoderIds={std::string(IsoFm::Constants::Id)};}
    else if(prefix(*format,Formats::EpsonQx10Prefix))policy.kind=PolicyKind::Epson;
    else if(same(*format,Formats::UcsdIbmMfm))policy.kind=PolicyKind::Ucsd;
    else policy.kind=PolicyKind::Generic;
    return policy;
}
std::shared_ptr<const MediaFileSystems::SectorImage> build(const Policy& policy,const std::optional<std::string>& format,const CandidateSet& candidates,std::stop_token token){
    checkCancellation(token);
    if(policy.kind==PolicyKind::Automatic)return buildAutomatic(candidates,token);
    if(!format)throw MediaError(DiagnosticCode::IsoRequestedGeometry,{std::string(RuntimeConstants::AutomaticSelection)});
    switch(policy.kind){
    case PolicyKind::AcornAdfs:return buildAdfs(*format,candidates,token);
    case PolicyKind::Atari8Bit:return buildAtari8Bit(candidates,format,token);
    case PolicyKind::AtariSt:return buildSt(candidates,format,token);
    case PolicyKind::Amstrad:return buildGeneric(*format,candidates,true,token);
    case PolicyKind::Ibm:return buildIbm(candidates,format,true,token);
    case PolicyKind::Bbc:return buildBbc(candidates,token,format);
    case PolicyKind::Epson:return buildEpson(*format,candidates.physical,token);
    case PolicyKind::Ucsd:return buildUcsd(*format,candidates,token);
    default:return buildGeneric(*format,candidates,false,token);
    }
}
}
