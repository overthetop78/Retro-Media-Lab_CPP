#include "WriterAdapters.h"
#include "WriterRegistry.h"
#include "../Models/Sectors/Representation.h"
#include "../Models/Flux/Representation.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return std::string(value.begin(),value.end());}
std::vector<std::string> distinct(std::vector<std::string> values){std::vector<std::string> result;for(auto& value:values)if(std::none_of(result.begin(),result.end(),[&](const auto& existing){return Primitives::ordinalIdentifierEqualsIgnoreCase(existing,value);}))result.push_back(std::move(value));return result;}
template<class Image,class Representation> class Adapter final:public ImageWriter {
    struct State {WriterCapabilities capabilities;TypedWriterPredicate<Image> canWrite;TypedWriterOutput<Image> write;WriterTargetPredicate target;};
    std::shared_ptr<const State> state_;
    static bool supports(const State& state,std::string_view format,std::string_view extension){const auto normalized=normalizeExtension(extension);const auto& caps=state.capabilities;return std::any_of(caps.formatIds.begin(),caps.formatIds.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,format);})&&std::any_of(caps.producedFileExtensions.begin(),caps.producedFileExtensions.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,normalized);})&&(!state.target||state.target(format,normalized));}
    static bool accepts(const State& state,const MediaDocument& document,std::string_view format,std::string_view extension){const auto normalized=normalizeExtension(extension);const auto* rep=dynamic_cast<const Representation*>(&document.representation());return rep&&supports(state,format,normalized)&&state.canWrite(rep->image(),document,format,normalized);}
public:
    Adapter(std::string id,RepresentationKind kind,std::vector<std::string> formats,std::vector<std::string> extensions,TypedWriterPredicate<Image> canWrite,TypedWriterOutput<Image> write,bool multiple,WriterTargetPredicate target){
        if(Primitives::isBlankIdentifier(id))throw M::MediaError(M::DiagnosticCode::WriterAdapterId);
        if(!canWrite||!write)throw M::MediaError(M::DiagnosticCode::WriterAdapterCallbacks);
        if(formats.empty()||std::any_of(formats.begin(),formats.end(),Primitives::isBlankIdentifier))throw M::MediaError(M::DiagnosticCode::WriterAdapterFormats);
        if(extensions.empty()||std::any_of(extensions.begin(),extensions.end(),Primitives::isBlankIdentifier))throw M::MediaError(M::DiagnosticCode::WriterAdapterExtensions);
        for(auto& extension:extensions)extension=normalizeExtension(extension);
        state_=std::make_shared<const State>(State{{std::move(id),distinct(std::move(formats)),distinct(std::move(extensions)),{kind},multiple},std::move(canWrite),std::move(write),std::move(target)});
    }
    const WriterCapabilities& capabilities()const override{return state_->capabilities;}
    bool supportsTarget(std::string_view format,std::string_view extension)const override{return supports(*state_,format,extension);}
    bool canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const override{return accepts(*state_,document,format,extension);}
    std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const override{return std::async(std::launch::deferred,[state=state_,document=std::move(document),output=std::move(output),target=std::move(target),token]{
        if(!document)throw M::MediaError(M::DiagnosticCode::DocumentNull);
        if(Primitives::isBlankIdentifier(utf8(output)))throw M::MediaError(M::DiagnosticCode::WriterAdapterPath);
        const auto* rep=dynamic_cast<const Representation*>(&document->representation());if(!rep)throw M::MediaError(M::DiagnosticCode::WriterAdapterRepresentation,{state->capabilities.id});
        const auto extension=utf8(output.extension());if(!accepts(*state,*document,target,extension))throw M::MediaError(M::DiagnosticCode::WriterAdapterTarget,{state->capabilities.id,target,extension});
        auto files=state->write(rep->image(),*document,output,target,token);if(files.empty()||std::any_of(files.begin(),files.end(),[](const auto& file){return Primitives::isBlankIdentifier(utf8(file));}))throw M::MediaError(M::DiagnosticCode::WriterAdapterProduced,{state->capabilities.id});return files;
    });}
};
}
std::shared_ptr<const ImageWriter> sectorWriterAdapter(std::string id,std::vector<std::string> formats,std::vector<std::string> extensions,TypedWriterPredicate<M::SectorImage> canWrite,TypedWriterOutput<M::SectorImage> write,bool multiple,WriterTargetPredicate target){return std::make_shared<Adapter<M::SectorImage,SectorRepresentation>>(std::move(id),RepresentationKind::Sectors,std::move(formats),std::move(extensions),std::move(canWrite),std::move(write),multiple,std::move(target));}
std::shared_ptr<const ImageWriter> fluxWriterAdapter(std::string id,std::vector<std::string> formats,std::vector<std::string> extensions,FluxWriterPredicate canWrite,FluxWriterOutput write,bool multiple){
    if(!canWrite||!write)throw M::MediaError(M::DiagnosticCode::WriterAdapterCallbacks);
    return std::make_shared<Adapter<Flux::ProtectedTrackImage,FluxRepresentation>>(std::move(id),RepresentationKind::Flux,std::move(formats),std::move(extensions),[predicate=std::move(canWrite)](const auto& image,const auto&,auto format,auto extension){return predicate(image,format,extension);},[writer=std::move(write)](const auto& image,const MediaDocument& document,const auto& output,auto target,auto token){return writer(image,document.metadata(),output,target,token);},multiple,WriterTargetPredicate{});
}
}
