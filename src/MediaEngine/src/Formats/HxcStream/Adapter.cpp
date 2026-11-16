#include "Adapter.h"
#include "Internal.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Sources/FileIdentity.h"
#include <QByteArray>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
std::string utf8(const std::filesystem::path& path){const auto text=path.u8string();return {text.begin(),text.end()};}
}
const ReaderCapabilities& Reader::capabilities()const {
    using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{std::string(Extension)},{MediaKind::Floppy},{RepresentationKind::Flux}};return caps;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;if((context.requestedFormatId()&&!Primitives::ordinalIdentifierEqualsIgnoreCase(*context.requestedFormatId(),FormatId))||!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Extension)||context.length()<std::int64_t(ChunkHeaderBytes+CrcBytes))return false;
        const auto bytes=context.readHeaderAsync(ChunkHeaderBytes,token).get();return readU32(bytes,0)==ChunkSignature;
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{using namespace Constants;const auto& primary=context.source()->primaryPath();const auto files=findTrackFiles(std::filesystem::path(std::u8string(primary.begin(),primary.end())));
        std::vector<Flux::ProtectedTrack> tracks;Metadata metadata;bool writeProtected=false;
        for(const auto& file:files){checkCancellation(token);const auto source=fileSource(file.path);if(source->length()>MaximumSignedSize)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::ContextFullRead);
            std::vector<std::uint8_t> bytes(std::size_t(source->length()));source->readExactly(0,bytes,token);auto decoded=readTrack(std::move(bytes),token);
            tracks.emplace_back(file.cylinder,file.head,std::nullopt,std::vector<Flux::TrackTimingSegment>{},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::move(decoded.revolutions));writeProtected|=decoded.writeProtected;
            const auto prefix=fieldsPrefix(file.cylinder,file.head);for(const auto& [key,value]:decoded.metadata)metadata[prefix+key]=value;
            metadata[nativeKey(file.cylinder,file.head)]=QByteArray(reinterpret_cast<const char*>(decoded.nativeBytes.data()),qsizetype(decoded.nativeBytes.size())).toBase64().toStdString();
        }
        for(const auto& track:tracks)metadata[digestKey(track.cylinder(),track.head())]=digest(track,writeProtected);
        SourceOptions options;options.primaryPath=primary;options.requestedFormatId=context.requestedFormatId();std::uint64_t total{};
        const auto original=fileIdentity(std::filesystem::path(std::u8string(primary.begin(),primary.end())));
        for(const auto& file:files){const auto identity=fileIdentity(file.path);if(!identity.exists)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(file.path)});
            if(identity.length>MaximumTime-total)throw MediaError(DiagnosticCode::HxcDuration);total+=identity.length;
            if(!Primitives::ordinalIdentifierEqualsIgnoreCase(utf8(identity.path),utf8(original.path)))options.associatedPaths.push_back(utf8(file.path));
        }
        options.knownLength=std::int64_t(total);return floppyFluxDocument(std::make_shared<const MediaSource>(std::move(options)),std::string(FormatId),std::make_shared<const Flux::ProtectedTrackImage>(std::move(tracks),writeProtected),std::move(metadata));
    });
}
}
