#include "Reader.h"
#include "Codec.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&sameIdentifier(document.formatId(),FormatId)&&document.mediaKind()==MediaKind::Tape&&dynamic_cast<const SequentialImage*>(&document.representation());}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{checkCancellation(token);const auto extension=path.extension().u8string();if(!document||!Writer{}.canWrite(*document,format,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::AtomTapWrite);const auto decoded=decode(dynamic_cast<const SequentialImage&>(document->representation()),token);if(!decoded.undecodedSegments().empty()||!decoded.diagnostics().empty()||!decoded.messages().empty())throw MediaError(DiagnosticCode::AtomTapWrite);const auto bytes=encode(decoded.blocks(),token);writeAtomicImage(path,[&](const auto& output,auto stop){checkCancellation(stop);output(bytes);},token);return std::vector<std::filesystem::path>{path};});}
}
