#include "Inspection.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
namespace RetroMediaLab::MediaEngine::ScpInspection {
std::vector<const Scp::Track*> sampleTracks(const Scp::Image& source){
 std::vector<const Scp::Track*> readable;for(const auto& track:source.tracks)if(!track.revolutions.empty())readable.push_back(&track);
 if(readable.size()<=Constants::MaximumTrackCount)return readable;
 std::vector<const Scp::Track*> result;std::set<std::uint8_t> numbers;
 for(std::size_t index=0;index<Constants::MaximumTrackCount;++index){const auto track=readable[index*(readable.size()-1)/(Constants::MaximumTrackCount-1)];if(numbers.insert(track->trackNumber).second)result.push_back(track);}return result;
}
std::set<ScpReconstruction::Family> probeFamilies(const Scp::Image& source,const Decoding::DecodeOperation& decode,const Progress& progress,std::stop_token token){
 checkCancellation(token);if(!decode)throw MediaError(DiagnosticCode::IsoDecoderMissing);const auto samples=sampleTracks(source);std::set<ScpReconstruction::Family> families;if(samples.empty())return families;
 for(std::size_t index=0;index<Constants::Probes.size();++index){
  checkCancellation(token);const auto& definition=Constants::Probes[index];const std::string detail(definition.decoderIds.front());
  if(progress)progress({ProgressKind::FormatProbeStarted,detail,index,Constants::Probes.size(),{},{},{}});
  std::atomic_bool recognized=false;std::atomic_size_t next=0;std::stop_source failureStop;std::stop_callback cancel(token,[&]{failureStop.request_stop();});std::mutex gate;std::exception_ptr failure;
  const auto run=[&]{try{
   while(!recognized.load()){
    checkCancellation(failureStop.get_token());const auto sample=next.fetch_add(1);if(sample>=samples.size())return;
    const auto flux=std::make_shared<const Flux::Revolution>(Scp::primaryWindow(*samples[sample],failureStop.get_token()).flux);
    for(const auto id:definition.decoderIds){checkCancellation(failureStop.get_token());const auto result=decode(id,flux);if(std::any_of(result.sectors.begin(),result.sectors.end(),[](const auto& sector){return sector.data&&sector.integrityValid==true;})){recognized=true;break;}}
   }
  }catch(...){std::lock_guard lock(gate);if(!failure)failure=std::current_exception();failureStop.request_stop();}};
  const auto parallelism=std::min(samples.size(),std::size_t(std::max(1u,std::thread::hardware_concurrency())));
  {std::vector<std::jthread> workers;workers.reserve(parallelism);for(std::size_t worker=0;worker<parallelism;++worker)workers.emplace_back(run);}
  if(failure)std::rethrow_exception(failure);checkCancellation(token);if(recognized)families.insert(definition.family);
  if(progress)progress({ProgressKind::FormatProbeCompleted,detail,index+1,Constants::Probes.size(),recognized.load(),{},{}});
 }
 return families;
}
}
