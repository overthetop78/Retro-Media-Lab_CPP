#pragma once
#include "../../../Composition/SequentialMedia.h"
#include "../../ReaderRegistry.h"
#include "../../WriterRegistry.h"

namespace RetroMediaLab::MediaEngine {
using ConversionMessages=std::vector<MediaFileSystems::MessageRecipe>;
class SequentialMediaConversionPlan final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialMediaConversionPlan(std::shared_ptr<const MediaDocument>,std::vector<std::string>,ConversionMessages);
    const std::shared_ptr<const MediaDocument>& targetDocument()const noexcept{return target_;}
    std::span<const std::string> diagnostics()const noexcept{return diagnostics_;}
    std::span<const MediaFileSystems::MessageRecipe> losses()const noexcept{return losses_;}
private:
    std::shared_ptr<const MediaDocument> target_;
    std::vector<std::string> diagnostics_;
    ConversionMessages losses_;
};
struct SequentialConversionDestination {
    std::string formatId,extension,writerId;
    bool producesMultipleFiles=false;
    bool operator==(const SequentialConversionDestination&)const=default;
};
struct SequentialConversionResult {
    std::vector<std::filesystem::path> producedFiles;
    std::vector<std::string> diagnostics;
    ConversionMessages losses;
};
class SequentialMediaConversionService final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialMediaConversionService(ReaderRegistry,SequentialMediaComposition,WriterRegistry);
    RETROMEDIALAB_MEDIAENGINE_API std::vector<SequentialConversionDestination> availableDestinations(std::shared_ptr<const MediaDocument>,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API SequentialMediaConversionPlan plan(std::shared_ptr<const MediaDocument>,std::string target,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API SequentialConversionResult convert(const std::filesystem::path& source,const std::filesystem::path& output,std::string target,std::optional<std::string> machine={},std::map<std::string,std::string> parameters={},bool acceptLosses=false,std::stop_token={})const;
private:
    std::optional<std::string> resolveMachine(std::shared_ptr<const MediaDocument>,std::optional<std::string>,std::stop_token)const;
    std::optional<SequentialEncodeRequest> selectRequest(const SequentialDecodeResult&,std::string target,std::string machine,const std::map<std::string,std::string>&,std::stop_token)const;
    ReaderRegistry readers_;
    SequentialMediaComposition codecs_;
    WriterRegistry writers_;
};
}
