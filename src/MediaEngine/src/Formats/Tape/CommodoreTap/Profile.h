#pragma once
#include "../../../Images/RecognitionContext.h"
namespace RetroMediaLab::MediaEngine::CommodoreTap {
struct Profile {std::string signature;std::uint8_t version{},platform{},video{};int clock{};};
std::optional<int> clockRate(std::uint8_t,std::uint8_t);
bool integer(const std::map<std::string,std::string>&,std::string_view,int&);
bool profile(const std::map<std::string,std::string>&,Profile&);
Profile readHeader(const RecognitionContext&,std::stop_token);
}
