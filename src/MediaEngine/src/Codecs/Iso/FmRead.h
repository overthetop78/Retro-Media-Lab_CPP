#pragma once
#include "Fm.h"
namespace RetroMediaLab::MediaEngine::IsoFm::Reading {
struct Header {std::uint64_t offset=0;std::uint8_t cylinder=0,head=0,sector=0,code=0;std::uint32_t size=0;std::optional<bool> valid;std::optional<std::vector<std::uint8_t>> bytes;};
struct Mark {std::uint64_t offset;std::uint8_t mark;bool deleted()const noexcept{return mark==Constants::DeletedMark;}};
struct Data {std::vector<std::uint8_t> payload,bytes;bool valid;};
struct Collection {std::vector<Header> headers;std::vector<Mark> data;};
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::uint8_t> recognize(Flux::BitView,std::int64_t);
RETROMEDIALAB_MEDIAENGINE_API Header header(Flux::BitView,std::uint64_t,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> data(Flux::BitView,const Mark&,std::uint32_t,std::stop_token = {});
Collection collect(Flux::BitView,std::int32_t period,std::stop_token);
}
