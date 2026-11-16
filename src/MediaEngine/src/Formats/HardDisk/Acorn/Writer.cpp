#include "Container.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageSetWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <QByteArray>
namespace RetroMediaLab::MediaEngine::AcornWinchester {
namespace {
std::vector<std::uint8_t> restore(const MediaDocument& document){
    using namespace Constants;const auto found=document.metadata().find(std::string(DescriptorKey));if(found==document.metadata().end())return {};
    return decodeDescriptor(found->second);
}
std::future<std::vector<std::filesystem::path>> writeFiles(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token,bool replace,bool preallocate){
    return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token,replace,preallocate]{using namespace Constants;
        if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto extension=output.extension().u8string();if(!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::AcornDatWrite);
        const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());auto descriptor=restore(*document);if(descriptor.size()<MinimumDescriptorSize)descriptor.resize(DescriptorSize);updateGeometryIfChanged(descriptor,*blocks.geometry(),blocks.capacity());
        const auto dat=std::filesystem::path(QFileInfo(QString::fromStdU16String(output.u16string())).absoluteFilePath().toStdU16String());auto dsc=dat;dsc.replace_extension(DescriptorExtension);
        const std::vector<ImageFileOutput> files{{dat,[&](const auto& write,auto stop){copyBlocks(blocks,write,stop);},preallocate?std::uint64_t(blocks.capacity()):0},{dsc,[&](const auto& write,auto stop){checkCancellation(stop);write(descriptor);}}};writeAtomicImageSet(files,token,replace);return std::vector<std::filesystem::path>{dat,dsc};
    });
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},true};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->geometry()&&supportsGeometry(*blocks->geometry())&&blocks->logicalBlockSize()==blocks->geometry()->sectorSize()&&blocks->capacity()==blocks->geometry()->capacity()&&blocks->hasCompleteReadableCoverage();
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return writeFiles(std::move(document),std::move(output),std::move(target),token,true,false);}
std::future<std::vector<std::filesystem::path>> Writer::createAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token,bool preallocate)const{return writeFiles(std::move(document),std::move(output),std::move(target),token,false,preallocate);}
}
