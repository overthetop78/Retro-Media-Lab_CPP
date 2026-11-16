#pragma once
#include "SignalDecoder.h"
namespace RetroMediaLab::MediaEngine::TzxSignal {
class Expansion final {
public:
    explicit Expansion(std::stop_token token):token_(token){}
    void block(std::uint8_t,const SequentialSegmentFields&,std::span<const std::uint8_t>);
    std::shared_ptr<const SequentialImage> finish();
private:
    void pulse(std::uint16_t,int,std::int64_t,std::string_view);
    void pause(std::uint16_t,std::int64_t);
    void data(const SequentialSegmentFields&,std::span<const std::uint8_t>,int,int,std::uint8_t,std::uint16_t,std::uint16_t);
    void append(SequentialSegmentFields);
    std::vector<SequentialSegment> segments_;SequentialTime elapsed_{};std::int64_t length_=0;std::stop_token token_;
};
}
