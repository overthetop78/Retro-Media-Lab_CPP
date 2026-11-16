#include "Adapter.h"
#include "Representation.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageSetWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
namespace RetroMediaLab::MediaEngine::MfmEmu {
namespace {
std::future<std::vector<std::filesystem::path>> writeFiles(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token,bool replace,bool preallocate){return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token,replace,preallocate]{checkCancellation(token);const auto extension=output.extension().u8string();if(!document||!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::MfmOutput);auto container=dynamic_cast<const Representation&>(document->representation()).container();const auto path=std::filesystem::path(QFileInfo(QString::fromStdU16String(output.u16string())).absoluteFilePath().toStdU16String());const std::vector<ImageFileOutput> files{{path,[container](const auto& write,auto stop){container->copyTo(write,stop);},preallocate?container->length():0}};writeAtomicImageSet(files,token,replace);return std::vector<std::filesystem::path>{path};});}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Flux},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&dynamic_cast<const Representation*>(&document.representation());}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string target,std::stop_token token)const{return writeFiles(std::move(document),std::move(path),std::move(target),token,true,false);}
std::future<std::vector<std::filesystem::path>> Writer::createAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::stop_token token,bool preallocate)const{return writeFiles(std::move(document),std::move(path),std::string(Constants::FormatId),token,false,preallocate);}
}
