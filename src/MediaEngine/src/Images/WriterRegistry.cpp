#include "WriterRegistry.h"
#include "Constants.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
std::string normalizeExtension(std::string_view extension){if(Primitives::isBlankIdentifier(extension))throw M::MediaError(M::DiagnosticCode::WriterTargetExtension);auto value=Primitives::utf8Identifier(extension).toLower().toUtf8().toStdString();if(value.front()!=ImageConstants::ExtensionSeparator)value.insert(value.begin(),ImageConstants::ExtensionSeparator);return value;}
WriterRegistry::WriterRegistry(std::vector<std::shared_ptr<const ImageWriter>> writers):writers_(std::move(writers)){
    if(std::any_of(writers_.begin(),writers_.end(),[](const auto& writer){return !writer;}))throw M::MediaError(M::DiagnosticCode::WriterRegistryNull);
    for(std::size_t first=0;first<writers_.size();++first)for(auto next=first+1;next<writers_.size();++next)if(Primitives::ordinalIdentifierEqualsIgnoreCase(writers_[first]->capabilities().id,writers_[next]->capabilities().id))throw M::MediaError(M::DiagnosticCode::WriterRegistryDuplicate,{writers_[first]->capabilities().id});
}
std::vector<std::shared_ptr<const ImageWriter>> WriterRegistry::findTargetCandidates(std::string_view format,std::string_view extension)const{
    if(Primitives::isBlankIdentifier(format))throw M::MediaError(M::DiagnosticCode::WriterTargetFormat);const auto normalized=normalizeExtension(extension);std::vector<std::shared_ptr<const ImageWriter>> result;for(const auto& writer:writers_)if(writer->supportsTarget(format,normalized))result.push_back(writer);return result;
}
std::shared_ptr<const ImageWriter> WriterRegistry::resolve(const MediaDocument& document,std::string_view format,std::string_view extension)const{
    const auto candidates=findTargetCandidates(format,extension);const auto normalized=normalizeExtension(extension);for(const auto& writer:candidates){const auto& kinds=writer->capabilities().representationKinds;if(std::find(kinds.begin(),kinds.end(),document.representationInfo().representationKind())!=kinds.end()&&writer->canWrite(document,format,normalized))return writer;}return {};
}
bool WriterRegistry::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{return bool(resolve(document,format,extension));}
}
