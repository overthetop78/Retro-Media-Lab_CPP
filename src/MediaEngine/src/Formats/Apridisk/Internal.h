#pragma once
#include "Constants.h"
#include "Apridisk.h"
namespace RetroMediaLab::MediaEngine::Apridisk::Internal {
template<class T>T little(MediaFileSystems::Bytes bytes,std::size_t at){T value=0;for(std::size_t i=0;i<sizeof(T);++i)value|=T(T(bytes[at+i])<<(i*Constants::BitsPerByte));return value;}
template<class T>void put(MediaFileSystems::MutableBytes bytes,std::size_t at,T value){for(std::size_t i=0;i<sizeof(T);++i)bytes[at+i]=std::uint8_t(value>>(i*Constants::BitsPerByte));}
}
