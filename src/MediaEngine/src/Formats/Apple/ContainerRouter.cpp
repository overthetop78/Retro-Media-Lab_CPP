#include "Adapter.h"
#include "Constants.h"
#include "Raw.h"
#include "../TwoImg/TwoImg.h"
#include "../TwoImg/Constants.h"
#include "../DiskCopy/DiskCopy.h"
#include "../Woz/Woz.h"
#include "../Woz/Constants.h"
#include "../Nib/Nib.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
template<class Signature> bool starts(MediaFileSystems::Bytes bytes,const Signature& signature){return bytes.size()>=signature.size()&&std::equal(signature.begin(),signature.end(),bytes.begin());}
}
std::shared_ptr<const MediaFileSystems::SectorImage> readContainer(MediaFileSystems::Bytes bytes,std::string_view extension,std::optional<std::string_view> requested,std::stop_token token){
    MediaFileSystems::checkCancellation(token);
    if(starts(bytes,TwoImg::Constants::Signature))return TwoImg::read(bytes,token);
    if(DiskCopy::hasPrivateWord(bytes))return DiskCopy::read(bytes,token);
    if(starts(bytes,Woz::Constants::Version1Signature)||starts(bytes,Woz::Constants::Version2Signature))return Woz::read(bytes,token);
    if(Primitives::ordinalIdentifierEqualsIgnoreCase(extension,Constants::Nib))return Nib::read(bytes,token);
    try{return readRaw(bytes,extension,token).image;}
    catch(const MediaError& error){
        if(error.diagnostic().code!=DiagnosticCode::RawUnknownGeometry&&error.diagnostic().code!=DiagnosticCode::AppleRawUnrecognized&&error.diagnostic().code!=DiagnosticCode::RawInvalidLength)throw;
        throw MediaError(DiagnosticCode::AppleContainerUnrecognized,{std::string(extension),std::string(requested.value_or(Constants::NoRequestedFormat))});
    }
}
}
