#include "ReaderRegistry.h"
#include "Constants.h"
#include "ReadRejection.h"
#include "../Sources/DriveAccess.h"
#include "../Sources/DriveOperation.h"
#include "../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace {
bool contains(std::span<const std::string> values,std::string_view sought){return std::any_of(values.begin(),values.end(),[&](const auto& value){return Primitives::ordinalIdentifierEqualsIgnoreCase(value,sought);});}
bool matchesSignature(const ReaderCapabilities& caps,const RecognitionContext& context,std::stop_token token){
    std::size_t maximum=0;for(const auto& signature:caps.signatures)maximum=std::max(maximum,signature.size());
    if(maximum==0||std::uint64_t(context.length())<maximum)return false;
    if(maximum>std::size_t(std::numeric_limits<std::int32_t>::max()))throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::ContextFullRead);
    const auto header=context.readHeaderAsync(std::int32_t(maximum),token).get();
    return std::any_of(caps.signatures.begin(),caps.signatures.end(),[&](const auto& signature){return signature.size()<=header.size()&&std::equal(signature.begin(),signature.end(),header.begin());});
}
std::string extension(std::string_view path){const auto suffix=QFileInfo(Primitives::utf8Identifier(path)).suffix();return suffix.isEmpty()?std::string{}:std::string(1,ImageConstants::ExtensionSeparator)+suffix.toUtf8().toStdString();}
}
ReaderRegistry::ReaderRegistry(std::vector<std::shared_ptr<const ImageReader>> readers):readers_(std::move(readers)){
    if(std::any_of(readers_.begin(),readers_.end(),[](const auto& reader){return !reader;}))throw MediaError(DiagnosticCode::ReaderRegistryNull);
}
std::future<std::vector<ReaderCandidate>> ReaderRegistry::selectCandidatesAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[readers=readers_,context,token]{
        std::vector<ReaderCandidate> result;
        for(const auto& reader:readers){
            MediaFileSystems::checkCancellation(token);std::vector<RecognitionEvidence> evidence;double confidence=0;
            const auto requested=context.requestedFormatId();const auto& caps=reader->capabilities();
            const auto add=[&](RecognitionEvidence reason,double weight){evidence.push_back(reason);confidence=std::max(confidence,weight);};
            if(requested&&!Primitives::isBlankIdentifier(*requested)){
                if(!reader->supportsFormatId(*requested))continue;
                add(RecognitionEvidence::ExplicitFormat,ImageConstants::ExplicitFormatConfidence);
            }
            if(contains(caps.extensions,context.extension()))add(RecognitionEvidence::FileExtension,ImageConstants::ExtensionConfidence);
            const auto& paths=context.source()->options().associatedPaths;
            if(std::any_of(paths.begin(),paths.end(),[&](const auto& path){return contains(caps.associatedFileExtensions,extension(path));}))add(RecognitionEvidence::AssociatedFile,ImageConstants::AssociatedFileConfidence);
            if(matchesSignature(caps,context,token))add(RecognitionEvidence::FileSignature,ImageConstants::SignatureConfidence);
            if(reader->canReadAsync(context,token).get())add(RecognitionEvidence::ReaderProbe,ImageConstants::ProbeConfidence);
            if(!evidence.empty())result.push_back(ReaderCandidate(reader,confidence,std::move(evidence)));
        }
        std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.confidence()!=b.confidence()?a.confidence()>b.confidence():a.evidence().size()>b.evidence().size();});
        return result;
    });
}
}

namespace RetroMediaLab::MediaEngine {
std::future<RecognitionResult> ReaderRegistry::recognizeAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[readers=readers_,context,token]{
        const auto candidates=ReaderRegistry(readers).selectCandidatesAsync(context,token).get();std::vector<ReaderFailure> failures;
        for(const auto& candidate:candidates){
            MediaFileSystems::checkCancellation(token);const auto& kinds=candidate.reader()->capabilities().mediaKinds;
            if(kinds.size()==1)context.ensureDrive(kinds.front());
            const auto drive=context.ownsDrive()?context.source()->options().drive:std::shared_ptr<DriveMechanics>{};
            DriveOperationScope operation(drive.get());
            DriveAccessScope access(drive.get(),drive?drive->state().position:DrivePosition{},false);
            std::shared_ptr<const MediaDocument> document;std::exception_ptr failure;
            try{document=candidate.reader()->readAsync(context,token).get();if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);}
            catch(...){failure=std::current_exception();}
            access.finish();operation.finish();
            if(!failure)return RecognitionResult(candidate.reader(),std::move(document),std::move(failures));
            if(!isImageReadRejection(failure))std::rethrow_exception(failure);
            const auto previous=std::find_if(failures.begin(),failures.end(),[&](const auto& entry){return entry.reader==candidate.reader();});
            if(previous==failures.end())failures.push_back({candidate.reader(),std::move(failure)});else previous->exception=std::move(failure);
        }
        if(!failures.empty())throw RecognitionError({DiagnosticCode::ReaderAllRejected,{}},std::move(failures));
        throw RecognitionError({DiagnosticCode::ReaderNoneAccepted,{context.source()->primaryPath()}},{});
    });
}
}
