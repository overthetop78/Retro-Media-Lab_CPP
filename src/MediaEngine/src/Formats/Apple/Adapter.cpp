#include "Adapter.h"
#include "Constants.h"
#include "TrackSelectionConstants.h"
#include "../FormatIds.h"
#include "../DiskCopy/DiskCopy.h"
#include "../DiskCopy/Constants.h"
#include "../TwoImg/Constants.h"
#include "../Woz/Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {
bool rejectedFormat(DiagnosticCode code){
    switch(code){
    case DiagnosticCode::AppleContainerUnrecognized:case DiagnosticCode::AppleRawUnrecognized:case DiagnosticCode::RawUnknownGeometry:case DiagnosticCode::RawInvalidLength:
    case DiagnosticCode::DiskCopyHeader:case DiagnosticCode::DiskCopyPrivateWord:case DiagnosticCode::DiskCopyPayload:case DiagnosticCode::DiskCopyTags:
    case DiagnosticCode::TwoImgSignature:case DiagnosticCode::TwoImgVersion:case DiagnosticCode::TwoImgHeader:case DiagnosticCode::TwoImgDataRange:case DiagnosticCode::TwoImgFormat:
    case DiagnosticCode::WozHeader:case DiagnosticCode::WozDiskType:case DiagnosticCode::WozMissingChunk:case DiagnosticCode::WozTruncatedChunk:case DiagnosticCode::WozTrackReference:case DiagnosticCode::WozCrc:case DiagnosticCode::AppleNoRwtsSectors:return true;
    default:return false;
    }
}
}
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities result{
        {std::string(Formats::AppleIIAppleDos113),std::string(Formats::AppleIIAppleDos140),std::string(Formats::AppleIIDos32),std::string(Formats::AppleIIDos33),std::string(TrackSelectionConstants::AppleIIGcr),std::string(Formats::AppleIIProDos),std::string(Formats::AppleIIProDos140),std::string(Formats::AppleIIProDos800),std::string(Formats::AppleIIRwts18),std::string(Formats::AppleIIISos),std::string(DiskCopy::Constants::AppleLisaMacWorks),std::string(Formats::AppleLisaOffice),std::string(Formats::AppleLisaRaw),std::string(Constants::MacGcr),std::string(Formats::AppleMacHfs),std::string(Formats::AppleMacMfs),std::string(Formats::Mac400),std::string(Formats::Mac800),std::string(Formats::Mac1440)},
        {std::string(Constants::TwoMg),std::string(Constants::D13),std::string(Constants::Dc42),std::string(Constants::Do),std::string(Constants::Dsk),std::string(Constants::Image),std::string(Constants::Img),std::string(Constants::Nib),std::string(Constants::Po),std::string(Constants::Woz)},
        {{TwoImg::Constants::Signature.begin(),TwoImg::Constants::Signature.end()},{Woz::Constants::Version1Signature.begin(),Woz::Constants::Version1Signature.end()},{Woz::Constants::Version2Signature.begin(),Woz::Constants::Version2Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    return result;
}
bool Reader::supportsFormatId(std::string_view id)const {
    constexpr std::string_view prefixes[]{Formats::AppleIIPrefix,Formats::AppleIIIPrefix,Formats::AppleLisaPrefix,Formats::AppleMacPrefix,Formats::MacPrefix};
    return ImageReader::supportsFormatId(id)||std::any_of(std::begin(prefixes),std::end(prefixes),[&](auto prefix){return id.size()>=prefix.size()&&Primitives::ordinalIdentifierEqualsIgnoreCase(id.substr(0,prefix.size()),prefix);});
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;
        const auto bytes=context.readBytesAsync(token).get();
        try{readContainer(bytes,context.extension(),requested?std::optional<std::string_view>(*requested):std::nullopt,token);return true;}
        catch(const MediaError& error){if(rejectedFormat(error.diagnostic().code))return false;throw;}
        catch(const MediaFileSystems::MediaError& error){if(error.kind()==MediaFileSystems::ErrorKind::InvalidData||error.kind()==MediaFileSystems::ErrorKind::Unsupported)return false;throw;}
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto bytes=context.readBytesAsync(token).get();
        if(DiskCopy::hasPrivateWord(bytes)){const auto image=DiskCopy::readDetailed(bytes,token);return floppySectorDocument(context.source(),image.image,DiskCopy::metadata(image));}
        const auto requested=context.requestedFormatId();return floppySectorDocument(context.source(),readContainer(bytes,context.extension(),requested?std::optional<std::string_view>(*requested):std::nullopt,token));
    });
}
}
