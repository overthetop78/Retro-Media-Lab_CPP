#include "SequentialContentAdapter.h"
#include "RecognitionContext.h"
#include "../Decoding/Sequential/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <RetroMediaLab/MediaFileSystems/SequentialContent.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <RetroMediaLab/MediaFileSystems/VolumeConstants.h>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
// These cases correspond to InvalidDataException in the C# decoder boundary.
bool invalidDecodedData(DiagnosticCode code) {
    switch(code){
    case DiagnosticCode::SequentialContentVolume:
    case DiagnosticCode::AtariTapeIncompatible:
    case DiagnosticCode::AcornTapeIncompatible:
    case DiagnosticCode::AtomTapeIncompatible:
    case DiagnosticCode::CpcTapeIncompatible:
    case DiagnosticCode::CommodoreTapeIncompatible:
    case DiagnosticCode::SpectrumTapeIncompatible:
    case DiagnosticCode::MsxTapeIncompatible:
    case DiagnosticCode::TzxSignalDocument:
    case DiagnosticCode::TzxSignalBits:
    case DiagnosticCode::TzxBlock:return true;
    default:return false;
    }
}
class Content final:public M::SequentialContent {
public:
    std::optional<std::string> decoder;std::vector<M::SequentialDecodedBlock> values;std::vector<std::string> raw;std::vector<M::MessageRecipe> recipes;
    std::optional<std::string> decoderId()const override{return decoder;}
    std::span<const M::SequentialDecodedBlock> blocks()const override{return values;}
    std::span<const std::string> diagnostics()const override{return raw;}
    std::span<const M::MessageRecipe> messages()const override{return recipes;}
};
}
SequentialContentAdapter::SequentialContentAdapter(SequentialDecoderRegistry registry):decoders_(std::move(registry)){}
std::string_view SequentialContentAdapter::id()const noexcept{return M::FileSystemIds::SequentialContent;}
bool SequentialContentAdapter::canRead(const M::ImageDocument& document,const M::VolumeDescriptor& volume,std::stop_token token)const {
    checkCancellation(token);const auto* native=dynamic_cast<const MediaDocument*>(&document);
    return native&&native->mediaKind()==MediaKind::Tape&&dynamic_cast<const SequentialImage*>(&document.representation())&&((volume.fileSystemId()&&sameIdentifier(*volume.fileSystemId(),id()))||sameIdentifier(volume.origin(),M::VolumeConstants::SequentialContent));
}
M::FileSystemVolume SequentialContentAdapter::read(const M::ImageDocument& document,const M::VolumeDescriptor& volume,std::stop_token token)const {
    try {
    if(!canRead(document,volume,token))throw MediaError(DiagnosticCode::SequentialContentVolume);
    const auto& native=dynamic_cast<const MediaDocument&>(document);std::vector<SequentialDecodeResult> results;
    for(const auto& decoder:decoders_.findCompatible(native,{},token)){checkCancellation(token);results.push_back(decoder->decodeAsync(native,{},token).get());}
    std::stable_sort(results.begin(),results.end(),[](const auto& left,const auto& right){
        if(left.blocks().empty()!=right.blocks().empty())return !left.blocks().empty();if(left.confidence()!=right.confidence())return left.confidence()>right.confidence();return left.blocks().size()>right.blocks().size();
    });
    Content content;
    if(!results.empty()) {
        const auto& selected=results.front();content.decoder=selected.decoderId();content.raw.assign(selected.diagnostics().begin(),selected.diagnostics().end());content.recipes.assign(selected.messages().begin(),selected.messages().end());
        if(results.size()>1)content.recipes.push_back({std::string(DiagnosticKeys::SequentialDecoderSelected),{selected.decoderId(),std::to_string(results.size())},{}});
        for(const auto& segment:selected.undecodedSegments()) {
            const auto kind=std::size_t(segment.fields().kind);const auto name=kind<SequentialDecodingConstants::SegmentNames.size()?std::string(SequentialDecodingConstants::SegmentNames[kind]):std::to_string(kind);
            content.recipes.push_back({std::string(DiagnosticKeys::SequentialUndecoded),{name,std::to_string(segment.fields().position)},{}});
        }
        for(const auto& block:selected.blocks()){checkCancellation(token);content.values.emplace_back(block.position(),std::vector<std::uint8_t>(block.data().begin(),block.data().end()),block.integrityValid(),block.metadata());}
    }
    const auto found=document.metadata().find(std::string(SequentialDecodingConstants::InternalNameKey));const auto name=found==document.metadata().end()?std::string{}:QString::fromStdString(found->second).trimmed().toStdString();
    return M::SequentialContentReader{}.read(document,volume,content,name,token);
    } catch(const MediaError& error) {
        if(!invalidDecodedData(error.diagnostic().code))throw;
        const auto& diagnostic=error.diagnostic();
        M::MessageRecipe message{std::string(diagnosticKey(diagnostic.code)),diagnostic.arguments,std::string(ImageConstants::EngineDomain)};
        throw M::MediaError(M::Diagnostic{M::DiagnosticCode::UnknownDiagnostic,diagnostic.arguments,{},{},std::move(message)},M::ErrorKind::InvalidData);
    }
}
}
