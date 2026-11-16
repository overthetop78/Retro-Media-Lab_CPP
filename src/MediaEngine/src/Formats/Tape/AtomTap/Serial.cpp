#include "Codec.h"
#include "Constants.h"
#include "../Uef/Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
SequentialDecodeResult decodeSerial(std::span<const std::uint8_t> serial,std::stop_token token){
    std::vector<SequentialSegment> segments;std::vector<bool> consumed(serial.size());const auto add=[&](std::span<const std::uint8_t> bytes){std::vector<std::uint8_t> data{DataBits,NoParity,StopBits};data.insert(data.end(),bytes.begin(),bytes.end());SequentialSegmentFields value;value.position=segments.size();value.kind=SequentialSegmentKind::DataBlock;value.dataRange=DataRange(0,data.size(),DataRangeKind::Stored,memorySource(data));value.metadata={{std::string(Uef::Constants::ChunkIdMetadataKey),std::string(DefinedChunk)}};segments.emplace_back(std::move(value));};
    for(std::size_t at=0;at<=serial.size()&&serial.size()-at>=SyncBytes;++at){checkCancellation(token);if(!std::all_of(serial.begin()+at,serial.begin()+at+SyncBytes,[](auto value){return value==Sync;}))continue;const auto endIt=std::find(serial.begin()+at+SyncBytes,serial.end(),NameTerminator);if(endIt==serial.end())continue;const auto end=static_cast<std::size_t>(endIt-serial.begin());if(end<at+SyncBytes+1||end>at+SyncBytes+FileNameBytes||serial.size()-end<HeaderTail)continue;const std::size_t count=(serial[end+FlagsAfterName]&ContainsData)?serial[end+CountAfterName]+CountBias:0;const auto payloadAt=end+HeaderTail;if(serial.size()-payloadAt<count+CountBias)continue;add(serial.subspan(at,payloadAt-at));add(serial.subspan(payloadAt,count+CountBias));std::fill(consumed.begin()+at,consumed.begin()+payloadAt+count+CountBias,true);at=payloadAt+count;}
    const auto decoded=decode(SequentialImage(std::nullopt,std::nullopt,std::move(segments)),token);auto messages=std::vector<MediaFileSystems::MessageRecipe>(decoded.messages().begin(),decoded.messages().end());if(std::any_of(consumed.begin(),consumed.end(),[](bool value){return !value;}))messages.push_back({std::string(DiagnosticKeys::AtomTapSerialResidual),{},{}});return SequentialDecodeResult(decoded.decoderId(),decoded.confidence(),{decoded.blocks().begin(),decoded.blocks().end()},{decoded.undecodedSegments().begin(),decoded.undecodedSegments().end()},{decoded.diagnostics().begin(),decoded.diagnostics().end()},std::move(messages));
}
}
