#pragma once
#include "Rx02.h"
namespace RetroMediaLab::MediaEngine::DecRx02 {
Decoding::LocalizedText headerDescription(const Header&,const DataMark*,std::optional<bool> dataValid);
Decoding::LocalizedText dataDescription(const Header&,const DataMark&,bool valid);
Decoding::LocalizedText unpairedDescription(const DataMark&);
}
