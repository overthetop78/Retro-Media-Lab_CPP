#include "Adapter.h"
#include "Internal.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Sources/FileIdentity.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::KryoFlux {
namespace {
std::string utf8(const std::filesystem::path& path){const auto text=path.u8string();return {text.begin(),text.end()};}
std::filesystem::path fromUtf8(const std::string& path){return std::filesystem::path(std::u8string(path.begin(),path.end()));}
}
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{std::string(Extension)},{MediaKind::Floppy},{RepresentationKind::Flux}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;checkCancellation(token);
        if((context.requestedFormatId()&&!Primitives::ordinalIdentifierEqualsIgnoreCase(*context.requestedFormatId(),FormatId))||!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Extension)||context.length()<MinimumRecognitionBytes)return false;
        const auto name=QFileInfo(QString::fromUtf8(context.source()->primaryPath())).fileName();
        if(!QRegularExpression(QString::fromUtf8(FilePattern),QRegularExpression::CaseInsensitiveOption|QRegularExpression::UseUnicodePropertiesOption).match(name).hasMatch())return false;
        return containsInfo(context.readHeaderAsync(std::int32_t(std::min(std::int64_t(RecognitionHeaderBytes),context.length())),token).get());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;checkCancellation(token);const auto& primary=context.source()->primaryPath();const auto files=findTrackFiles(fromUtf8(primary),token);
        std::vector<Flux::ProtectedTrack> tracks;Metadata metadata;SourceOptions options;options.primaryPath=primary;options.requestedFormatId=context.requestedFormatId();std::uint64_t total{};
        const auto original=fileIdentity(fromUtf8(primary));
        for(const auto& file:files){
            checkCancellation(token);const auto source=fileSource(file.path);if(source->length()>std::numeric_limits<std::int32_t>::max())throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::ContextFullRead);
            std::vector<std::uint8_t> bytes(std::size_t(source->length()));source->readExactly(0,bytes,token);auto decoded=readTrack(bytes,token);
            tracks.emplace_back(file.cylinder,file.head,std::nullopt,std::vector<Flux::TrackTimingSegment>{},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::move(decoded.revolutions));
            for(auto& [key,value]:decoded.metadata)metadata.try_emplace(key,std::move(value));
            const auto identity=fileIdentity(file.path);if(!identity.exists)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(file.path)});
            if(identity.length>MaximumTotalLength-total)throw MediaError(DiagnosticCode::KryoDuration);total+=identity.length;
            if(!Primitives::ordinalIdentifierEqualsIgnoreCase(utf8(identity.path),utf8(original.path)))options.associatedPaths.push_back(utf8(file.path));
        }
        options.knownLength=std::int64_t(total);return floppyFluxDocument(std::make_shared<const MediaSource>(std::move(options)),std::string(FormatId),std::make_shared<const Flux::ProtectedTrackImage>(std::move(tracks),true),std::move(metadata));
    });
}
}
