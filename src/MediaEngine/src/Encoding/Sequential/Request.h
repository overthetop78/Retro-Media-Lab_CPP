#pragma once
#include "../../Decoding/Sequential/Models.h"
namespace RetroMediaLab::MediaEngine {
class SequentialEncodeRequest final {
public:
    RETROMEDIALAB_MEDIAENGINE_API SequentialEncodeRequest(std::string target,std::string machine,std::vector<SequentialDecodedBlock> blocks,std::vector<SequentialSegment> retained={},std::map<std::string,std::string> parameters={});
    const std::string& targetFormatId()const noexcept{return target_;}
    const std::string& machineId()const noexcept{return machine_;}
    std::span<const SequentialDecodedBlock> blocks()const noexcept{return blocks_;}
    std::span<const SequentialSegment> retainedSegments()const noexcept{return retained_;}
    const std::map<std::string,std::string>& parameters()const noexcept{return parameters_;}
private:
    std::string target_,machine_;std::vector<SequentialDecodedBlock> blocks_;std::vector<SequentialSegment> retained_;std::map<std::string,std::string> parameters_;
    friend class SequentialEncoderRegistry;
};
}
