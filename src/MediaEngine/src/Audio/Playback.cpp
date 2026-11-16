#include <RetroMediaLab/MediaEngine/PcmPlaybackSession.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "PlaybackConstants.h"
#include "../Composition/Recognition.h"
#include "../Formats/Common/Audio/Constants.h"
#include "../Formats/Tape/Common/PcmConstants.h"
#include "../Models/Sequential/Image.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <utility>
namespace RetroMediaLab::MediaEngine {
namespace C=PlaybackConstants;namespace A=AudioPcm::Constants;namespace P=TapePcm::Constants;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::AudioPcmData);}
int number(const MediaDocument& document,std::string_view key){const auto entry=document.metadata().find(std::string(key));if(entry==document.metadata().end())invalid();bool ok=false;const auto result=Primitives::utf8Identifier(entry->second).toInt(&ok,C::DecimalBase);if(!ok||result<=0)invalid();return result;}
void resetFailedDocument(const std::shared_ptr<const MediaDocument>& document)noexcept{try{if(document&&document->mediaSource()->options().drive)document->mediaSource()->options().drive->reset();}catch(...){}}
}
struct PcmPlaybackSession::State {
 std::shared_ptr<const MediaDocument> document;
 std::shared_ptr<const MediaFileSystems::RandomAccessData> source;
 std::int64_t offset=0,frames=0,position=0;
 int channels=0,rate=0;bool closed=false;
 std::array<std::uint8_t,C::ScratchBytes> scratch{};
};
PcmPlaybackSession::PcmPlaybackSession(std::unique_ptr<State> state):state_(std::move(state)){}
PcmPlaybackSession::~PcmPlaybackSession(){try{close();}catch(...){}}
std::unique_ptr<PcmPlaybackSession> PcmPlaybackSession::create(std::shared_ptr<const MediaDocument> document){
 try{
  if(!document)invalid();const auto* sequential=dynamic_cast<const SequentialImage*>(&document->representation());
  if(!sequential||!sequential->segments()||sequential->segments()->empty())invalid();
  const auto& range=sequential->segments()->front().fields().dataRange;if(!range||!range->source())invalid();
  auto state=std::make_unique<State>();state->channels=number(*document,P::ChannelsKey);state->rate=number(*document,P::RateKey);
  const auto bits=document->metadata().find(std::string(P::BitsKey));if(bits==document->metadata().end()||bits->second!=std::to_string(A::SampleBits))invalid();
  const auto align=std::int64_t(state->channels)*A::SampleBytes;
  if(range->length()<0||range->sourceOffset()<0||range->length()%align||std::uint64_t(range->sourceOffset())>range->source()->length()||std::uint64_t(range->length())>range->source()->length()-std::uint64_t(range->sourceOffset()))invalid();
  state->frames=range->length()/align;state->offset=range->sourceOffset();state->source=range->source();state->document=document;
  return std::unique_ptr<PcmPlaybackSession>(new PcmPlaybackSession(std::move(state)));
 }catch(...){resetFailedDocument(document);throw;}
}
std::future<std::unique_ptr<PcmPlaybackSession>> PcmPlaybackSession::openAsync(std::filesystem::path path,std::stop_token token){return std::async(std::launch::async,[path=std::move(path),token]{checkCancellation(token);auto text=path.u8string();SourceOptions source;source.primaryPath=std::string(text.begin(),text.end());if(Primitives::isBlankIdentifier(source.primaryPath))invalid();source.requestedFormatId=std::string(A::Format);auto document=MediaRecognitionComposition::createDefault().readingService().readAsync(std::make_shared<MediaSource>(std::move(source)),token).get();return create(std::move(document));});}
int PcmPlaybackSession::channels()const noexcept{return state_->channels;}
int PcmPlaybackSession::sampleRate()const noexcept{return state_->rate;}
std::int64_t PcmPlaybackSession::framePosition()const noexcept{return state_->position;}
std::int64_t PcmPlaybackSession::frameCount()const noexcept{return state_->frames;}
std::size_t PcmPlaybackSession::readSamples(std::span<std::int16_t> destination,std::stop_token token){
 if(state_->closed)invalid();checkCancellation(token);
 const auto frames=std::min(std::uint64_t(destination.size()/state_->channels),static_cast<std::uint64_t>(state_->frames-state_->position));const auto samples=static_cast<std::size_t>(frames)*state_->channels;
 std::size_t completed=0;while(completed<samples){const auto count=std::min(samples-completed,state_->scratch.size()/A::SampleBytes);const auto bytePosition=static_cast<std::uint64_t>(state_->offset)+static_cast<std::uint64_t>(state_->position)*state_->channels*A::SampleBytes+completed*A::SampleBytes;
  state_->source->readExactly(bytePosition,std::span(state_->scratch).first(count*A::SampleBytes),token);
  for(std::size_t i=0;i<count;++i){const auto value=std::uint16_t(std::uint16_t(state_->scratch[i*A::SampleBytes])|(std::uint16_t(state_->scratch[i*A::SampleBytes+1])<<C::ByteBits));destination[completed+i]=std::bit_cast<std::int16_t>(value);}completed+=count;
 }state_->position+=static_cast<std::int64_t>(frames);return samples;
}
void PcmPlaybackSession::seekFrame(std::int64_t position){if(state_->closed||position<0||position>state_->frames)invalid();state_->position=position;}
void PcmPlaybackSession::close(){if(state_->closed)return;state_->closed=true;auto document=std::move(state_->document);auto drive=document?document->mediaSource()->options().drive:nullptr;state_->source.reset();document.reset();if(drive)drive->reset();}
}
