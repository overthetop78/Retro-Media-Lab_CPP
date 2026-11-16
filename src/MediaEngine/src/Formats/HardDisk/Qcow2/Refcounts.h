#pragma once
#include "Container.h"
#include <map>
namespace RetroMediaLab::MediaEngine::Qcow2 {
class Refcounts final {
public:
    Refcounts(RecognitionContext,const Header&,std::stop_token);
    void requireAllocated(std::uint64_t offset,std::stop_token);
private:
    RecognitionContext context_;Header header_;std::vector<std::uint8_t> table_;std::map<std::uint64_t,std::vector<std::uint8_t>> blocks_;
};
}
