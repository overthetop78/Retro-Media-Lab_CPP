#pragma once
#include "../MediaDocument.h"
#include <functional>
namespace RetroMediaLab::MediaEngine {
using RepresentationMessages=std::vector<MediaFileSystems::MessageRecipe>;
struct RepresentationConversionProgress {MediaFileSystems::MessageRecipe message;double percent;MediaKind mediaKind;};
using RepresentationProgress=std::function<void(const RepresentationConversionProgress&)>;
using ConversionOptions=std::map<std::string,std::string>;
class RepresentationConversionResult final {
public:
 RETROMEDIALAB_MEDIAENGINE_API RepresentationConversionResult(std::shared_ptr<const MediaDocument>,RepresentationMessages={},RepresentationMessages={});
 const std::shared_ptr<const MediaDocument>& document()const noexcept{return document_;}
 std::span<const MediaFileSystems::MessageRecipe> diagnostics()const noexcept{return diagnostics_;}
 std::span<const MediaFileSystems::MessageRecipe> losses()const noexcept{return losses_;}
private:
 std::shared_ptr<const MediaDocument> document_;RepresentationMessages diagnostics_,losses_;
};
class RETROMEDIALAB_MEDIAENGINE_NOVTABLE RepresentationConverter {
public:
 virtual ~RepresentationConverter()=default;
 virtual std::string_view id()const noexcept=0;
 virtual std::span<const RepresentationKind> sourceKinds()const noexcept=0;
 virtual std::span<const RepresentationKind> targetKinds()const noexcept=0;
 virtual bool canConvert(const MediaDocument&,std::string_view,RepresentationKind)const=0;
 // Synchronous worker API. No output files are created by these converters.
 virtual RepresentationConversionResult convert(std::shared_ptr<const MediaDocument>,std::string target,RepresentationKind,const ConversionOptions& = {},const RepresentationProgress& = {},std::stop_token={})const=0;
};
class RepresentationConverterRegistry final {
public:
 RETROMEDIALAB_MEDIAENGINE_API explicit RepresentationConverterRegistry(std::vector<std::shared_ptr<const RepresentationConverter>>);
 RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const RepresentationConverter> resolve(const MediaDocument&,std::string_view,RepresentationKind)const;
 std::span<const std::shared_ptr<const RepresentationConverter>> converters()const noexcept{return converters_;}
private:std::vector<std::shared_ptr<const RepresentationConverter>> converters_;
};
}
