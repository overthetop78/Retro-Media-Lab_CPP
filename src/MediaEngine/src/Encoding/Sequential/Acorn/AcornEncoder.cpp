#include "AcornEncoder.h"
#include "AcornOutput.h"
#include "../../../Formats/Tape/Acorn/Blocks.h"
#include "../../../Formats/Tape/Acorn/Constants.h"
#include "../../../Formats/Tape/Uef/Constants.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
using namespace AcornTape::Constants;using namespace Uef::Constants;
std::string_view AcornTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> AcornTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(FormatId),std::string(WaveFormat),std::string(CswFormat),std::string(RawFormat)};return ids;}
std::span<const std::string> AcornTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AcornTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{
    if(!std::any_of(formatIds().begin(),formatIds().end(),[&](const auto& id){return sameIdentifier(request.targetFormatId(),id);})||!sameIdentifier(request.machineId(),MachineId)||request.blocks().empty()||!request.retainedSegments().empty())return false;
    if(!std::all_of(request.blocks().begin(),request.blocks().end(),[](const auto& block){return block.integrityValid()==true&&AcornTape::canBuildBlock(block);}))return false;
    int rate{};return sameIdentifier(request.targetFormatId(),FormatId)||TapePcm::trySampleRate(request.parameters(),rate);
}
std::future<std::shared_ptr<const SequentialImage>> AcornTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{
    return std::async(std::launch::deferred,[request=std::move(request),token]{
        checkCancellation(token);if(!AcornTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::AcornTapeEncode);
        std::vector<std::vector<std::uint8_t>> blocks;for(const auto& block:request.blocks())blocks.push_back(AcornTape::buildBlock(block,token));
        if(sameIdentifier(request.targetFormatId(),FormatId))return AcornTape::encodeUef(blocks,token);
        int rate{};TapePcm::trySampleRate(request.parameters(),rate);return AcornTape::encodeWave(blocks,rate,token);
    });
}
}
