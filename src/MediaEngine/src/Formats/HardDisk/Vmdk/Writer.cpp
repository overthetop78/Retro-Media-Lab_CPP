#include "Writing.h"
#include "Descriptor.h"
#include "../../../Primitives/Identifiers.h"
#include "../../../Sources/AtomicImageSetWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vmdk {
namespace {
bool selectFixed(const MediaDocument& document,const BlockImage& blocks){
    using namespace Constants;const auto found=document.metadata().find(std::string(VariantKey));if(sameIdentifier(document.formatId(),FormatId)&&found!=document.metadata().end()){const auto text=QString::fromUtf8(found->second).trimmed();const auto value=text.toUtf8().toStdString();bool numeric=false;const auto number=text.toInt(&numeric);if(Primitives::ordinalIdentifierEqualsIgnoreCase(value,FixedVariant)||(numeric&&number==FixedEnum))return true;if(Primitives::ordinalIdentifierEqualsIgnoreCase(value,SparseVariant)||(numeric&&number==SparseEnum))return false;}
    return std::none_of(blocks.ranges().begin(),blocks.ranges().end(),[](const auto& range){return range.kind()==DataRangeKind::Zero||range.kind()==DataRangeKind::Unallocated;});
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},true};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&blocks->logicalBlockSize()==Constants::SectorSize&&blocks->hasCompleteReadableCoverage();}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token]{using namespace Constants;checkCancellation(token);const auto extension=output.extension().u8string();if(!document||!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::VmdkWrite);const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());
    if(!selectFixed(*document,blocks)){writeAtomicSeekableImage(output,[&](const auto& stream,auto stop){writeSparse(blocks,output.filename(),stream,stop);},token);return std::vector<std::filesystem::path>{output};}
    const auto full=std::filesystem::path(QFileInfo(QString::fromStdU16String(output.u16string())).absoluteFilePath().toStdU16String());auto flat=full.parent_path()/(full.stem().u16string()+std::filesystem::path(FlatSuffix).u16string()+std::filesystem::path(Extension).u16string());const auto descriptor=createDescriptor(blocks,MonolithicFlat,Flat,flat.filename(),FlatExtentSuffix);
    const std::vector<ImageFileOutput> files{{flat,[&](const auto& write,auto stop){copyBlocks(blocks,write,stop);}},{output,[&](const auto& write,auto stop){checkCancellation(stop);write(descriptor);}}};writeAtomicImageSet(files,token);return std::vector<std::filesystem::path>{output,flat};
});}
}
