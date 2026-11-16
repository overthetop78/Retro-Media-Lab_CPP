#pragma once
#include "Container.h"
#include <QString>
namespace RetroMediaLab::MediaEngine::Vmdk {
void validateDescriptor(const QString&,std::string_view expectedType);
std::shared_ptr<const MediaDocument> readDescriptor(const RecognitionContext&,bool includeRanges,std::stop_token);
std::vector<std::uint8_t> createDescriptor(const BlockImage&,std::string_view createType,std::string_view extentType,const std::filesystem::path& fileName,std::string_view suffix);
}
