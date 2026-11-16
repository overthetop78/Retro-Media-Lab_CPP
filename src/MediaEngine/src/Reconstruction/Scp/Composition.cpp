#include "Composition.h"
#include "Constants.h"
#include "../Iso/Reader.h"
#include "../Apple/Reader.h"
#include "../AmigaReader.h"
#include "../Rx02Reader.h"
#include "../DefinedReader.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Formats/CommodoreDisks/FluxReader.h"
#include "../../Formats/Epson/Geometry.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
namespace {
using Primitives::asciiIdentifierEqualsIgnoreCase;
using Primitives::asciiIdentifierStartsWithIgnoreCase;
struct Services {
    Decoding::Registry decoders;
    IsoReconstruction::ReadCapture capture;
    IsoReconstruction::Reader iso;
    AppleScp::Reader apple;
    DefinedReconstruction::Reader defined;
    Services(std::shared_ptr<const Scp::CaptureReader> source,Decoding::Registry registry)
        :decoders(std::move(registry)),capture([source=std::move(source)](const std::filesystem::path& path,std::stop_token token){return *source->readFileAsync(path,token).get();}),
         iso(decoders.operation(),capture),apple(decoders,capture),defined(decoders,capture){}
};
Candidates create(std::shared_ptr<const Scp::CaptureReader> capture,Decoding::Registry decoders){
    if(!capture)throw MediaError(DiagnosticCode::ReaderRegistryNull);
    const auto services=std::make_shared<Services>(std::move(capture),std::move(decoders));
    const auto isoCandidate=[&](std::string_view id,std::optional<std::string> fixed,bool automatic=false){
        return Candidate{std::string(id),Family::Iso,
            [services,fixed,automatic](const auto& path,const auto& format,const auto& progress,auto token){return services->iso.read(path,automatic?std::nullopt:fixed?fixed:format,progress,token);},
            [services,fixed,automatic](const auto& image,const auto& format,const auto& progress,auto token){return services->iso.read(image,automatic?std::nullopt:fixed?fixed:format,progress,token);}};
    };
    const auto isoFixed=[&](std::string_view format){return isoCandidate(std::string(Constants::IsoPrefix)+std::string(format),std::string(format));};
    const auto isoAutomatic=isoCandidate(Constants::IsoAutomatic,{},true),isoSelected=isoCandidate(Constants::IsoSelected,{});
    const Candidate defined{std::string(Constants::Defined),Family::Iso,
        [services](const auto& path,const auto& format,const auto& progress,auto token){return services->defined.read(path,format.value(),progress,token);},
        [services](const auto& image,const auto& format,const auto& progress,auto token){return services->defined.read(image,format.value(),progress,token);}};
    const Candidate amiga{std::string(Constants::Amiga),Family::Amiga,
        [services](const auto& path,const auto&,const auto& progress,auto token){return AmigaReconstruction::read(services->capture(path,token),services->decoders.operation(),token,progress);},
        [services](const auto& image,const auto&,const auto& progress,auto token){return AmigaReconstruction::read(Scp::fromProtected(image,{},token),services->decoders.operation(),token,progress);}};
    const Candidate atari{std::string(Constants::Atari),Family::Iso,
        [services](const auto& path,const auto& format,const auto& progress,auto token){return services->iso.readAtari(path,format,progress,token);},
        [services](const auto& image,const auto& format,const auto& progress,auto token){return services->iso.read(image,format,progress,token);}};
    auto atariSt=isoCandidate(Constants::AtariSt720,std::string(Formats::AtariSt720));
    const auto commodore=[&](bool fixed){return Candidate{std::string(fixed?Constants::Commodore1581:Constants::CommodoreAutomatic),fixed?Family::Iso:Family::Commodore,
        [services,fixed](const auto& path,const auto&,const Progress& progress,auto token){
            CommodoreDisks::TrackProgress callback;
            if(progress)callback=[progress](const Scp::Track& track,std::size_t done,std::size_t total){progress({IsoReconstruction::CollectionProgressKind::TrackDecoded,std::int32_t(done),std::int32_t(total),track.cylinder,track.head,0,std::int32_t(track.revolutions.size())});};
            return CommodoreDisks::readFlux(services->capture(path,token),fixed?std::optional<std::string_view>(Formats::Commodore1581):std::nullopt,callback,token);
        },
        [fixed](const auto& image,const std::optional<std::string>& format,const Progress& progress,auto token){
            CommodoreDisks::TrackProgress callback;
            if(progress)callback=[progress](const Scp::Track& track,std::size_t done,std::size_t total){progress({IsoReconstruction::CollectionProgressKind::TrackDecoded,std::int32_t(done),std::int32_t(total),track.cylinder,track.head,0,std::int32_t(track.revolutions.size())});};
            return CommodoreDisks::readFlux(Scp::fromProtected(image,{},token),fixed?std::optional<std::string_view>(Formats::Commodore1581):format?std::optional<std::string_view>(*format):std::nullopt,callback,token);
        }};};
    const auto cbm=commodore(false),cbm1581=commodore(true);
    const Candidate apple{std::string(Constants::Apple),Family::Apple,
        [services](const auto& path,const auto& format,const auto& progress,auto token){return services->apple.read(path,format,progress,token);},
        [services](const auto& image,const auto& format,const auto& progress,auto token){return services->apple.read(Scp::fromProtected(image,{},token),format,progress,token);}};
    const Candidate dec{std::string(Constants::Dec),Family::Dec,
        [services](const auto& path,const auto&,const auto& progress,auto token){return Rx02Reconstruction::read(services->capture(path,token),services->decoders.operation(),progress,token);},
        [services](const auto& image,const auto&,const auto& progress,auto token){return Rx02Reconstruction::read(Scp::fromProtected(image,{},token),services->decoders.operation(),progress,token);}};
    const auto adfs=isoFixed(Formats::AcornAdfs800),cpc=isoFixed(Formats::AmstradCpc),pcw=isoFixed(Formats::AmstradPcw),ibm=isoFixed(Formats::IbmScan),ucsd=isoFixed(Formats::UcsdIbmMfm);
    std::vector<Candidate> isoFamily{isoAutomatic,atariSt,adfs,cpc,pcw,ibm,ucsd,cbm1581};
    std::vector<Candidate> defaults{isoAutomatic,amiga,cbm1581,cbm,cpc,pcw,ibm};
    for(const auto& format:Epson::scpCandidateFormatIds()){auto candidate=isoFixed(format);isoFamily.push_back(candidate);defaults.push_back(std::move(candidate));}
    defaults.push_back(apple);
    const auto equal=[](std::string_view format){return [format](std::string_view id){return asciiIdentifierEqualsIgnoreCase(id,format);};};
    const auto prefix=[](std::string_view format){return [format](std::string_view id){return asciiIdentifierStartsWithIgnoreCase(id,format);};};
    std::vector<Selection> selections{
        {equal(Formats::Jfd),isoAutomatic},
        {[](std::string_view id){return Definitions::rawDefinitions().contains(id);},defined},
        {equal(Formats::ApricotPcXi315),isoSelected},
        {prefix(Formats::AmigaPrefix),amiga},{prefix(Formats::CommodorePrefix),cbm},
        {equal(Formats::AmstradCpc),cpc},{equal(Formats::AmstradPcw),pcw},
        {[](std::string_view id){return asciiIdentifierStartsWithIgnoreCase(id,Formats::IbmPrefix)||asciiIdentifierEqualsIgnoreCase(id,Formats::Mac1440);},isoSelected},
        {[](std::string_view id){return asciiIdentifierEqualsIgnoreCase(id,Formats::AcornDosPlus800)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AcornDfsPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AcornAdfsPrefix);},isoSelected},
        {equal(Formats::DecRx02),dec},
        {[](std::string_view id){return asciiIdentifierStartsWithIgnoreCase(id,Formats::EpsonQx10Prefix)||asciiIdentifierEqualsIgnoreCase(id,Formats::UcsdIbmMfm);},isoSelected},
        {[](std::string_view id){return asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariStPrefix);},atari},
        {[](std::string_view id){return asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIIPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleLisaPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleMacPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::MacPrefix);},apple}};
    return Candidates(std::move(selections),std::move(defaults),{{Family::Iso,std::move(isoFamily)},{Family::Amiga,{amiga}},{Family::Commodore,{cbm}},{Family::Apple,{apple}},{Family::Dec,{dec}}},{Family::Amiga,Family::Iso,Family::Commodore,Family::Apple,Family::Dec},isoSelected);
}
}
Composition::Composition(std::shared_ptr<const Scp::CaptureReader> capture,MediaFileSystems::SectorRegistry fileSystems,Decoding::Registry decoders)
    :candidates_(create(std::move(capture),std::move(decoders))),reader_(candidates_,std::move(fileSystems)){}
}
