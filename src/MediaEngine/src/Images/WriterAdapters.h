#pragma once
#include "Writers.h"
#include "../Models/Flux/ProtectedTrack.h"
#include <functional>
namespace RetroMediaLab::MediaEngine {
using WriterTargetPredicate=std::function<bool(std::string_view,std::string_view)>;
template<class Image> using TypedWriterPredicate=std::function<bool(const Image&,const MediaDocument&,std::string_view,std::string_view)>;
template<class Image> using TypedWriterOutput=std::function<std::vector<std::filesystem::path>(const Image&,const MediaDocument&,const std::filesystem::path&,std::string_view,std::stop_token)>;
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> sectorWriterAdapter(std::string id,std::vector<std::string> formats,std::vector<std::string> extensions,TypedWriterPredicate<MediaFileSystems::SectorImage>,TypedWriterOutput<MediaFileSystems::SectorImage>,bool multipleFiles=false,WriterTargetPredicate={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> fluxWriterAdapter(std::string id,std::vector<std::string> formats,std::vector<std::string> extensions,TypedWriterPredicate<Flux::ProtectedTrackImage>,TypedWriterOutput<Flux::ProtectedTrackImage>,bool multipleFiles=false);
using FluxWriterPredicate=std::function<bool(const Flux::ProtectedTrackImage&,std::string_view,std::string_view)>;
using FluxWriterOutput=std::function<std::vector<std::filesystem::path>(const Flux::ProtectedTrackImage&,const std::map<std::string,std::string>&,const std::filesystem::path&,std::string_view,std::stop_token)>;
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ImageWriter> fluxWriterAdapter(std::string id,std::vector<std::string> formats,std::vector<std::string> extensions,FluxWriterPredicate,FluxWriterOutput,bool multipleFiles=false);
}
