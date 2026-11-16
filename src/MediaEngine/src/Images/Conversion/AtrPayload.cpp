#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Formats/Atr/Atr.h"
#include "../../Formats/Atr/Constants.h"
#include "../RecognitionContext.h"
#include "../../Sources/AtomicImageWriter.h"
#include "../../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine {
std::future<void> extractAtrPayloadAsync(std::filesystem::path source,std::filesystem::path output,std::stop_token token){return std::async(std::launch::async,[source=std::move(source),output=std::move(output),token]{
 checkCancellation(token);const auto inputText=source.u8string(),outputText=output.u8string();
 if(Primitives::isBlankIdentifier(std::string(inputText.begin(),inputText.end()))||Primitives::isBlankIdentifier(std::string(outputText.begin(),outputText.end())))throw MediaError(DiagnosticCode::DiskPreparationRequest);
 SourceOptions options;options.primaryPath=std::string(inputText.begin(),inputText.end());RecognitionContext context(std::make_shared<MediaSource>(std::move(options)));
 const auto bytes=context.readBytesAsync(token).get();Atr::validate(bytes);checkCancellation(token);
 writeAtomicImage(output,[&](const ImageOutput& write,std::stop_token current){checkCancellation(current);write(std::span(bytes).subspan(Atr::Constants::HeaderSize));},token);
 });}
}
