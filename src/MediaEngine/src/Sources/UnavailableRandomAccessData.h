#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine {
class UnavailableRandomAccessData final:public MediaFileSystems::RandomAccessData {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit UnavailableRandomAccessData(std::int64_t);
    std::uint64_t length()const noexcept override{return length_;}
    RETROMEDIALAB_MEDIAENGINE_API void readExactly(std::uint64_t,MediaFileSystems::MutableBytes,std::stop_token={})const override;
private:std::uint64_t length_;
};
}
