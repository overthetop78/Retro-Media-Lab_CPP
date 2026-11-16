#pragma once
#include "PcmConstants.h"
#include "../../../Models/Sequential/Image.h"
#include "../../../Images/MediaDocument.h"
namespace RetroMediaLab::MediaEngine::TapePcm {
using Parameters=std::map<std::string,std::string>;
struct Profile {int sampleRate=Constants::DefaultRate,channels=Constants::DefaultChannels,bits=Constants::DefaultBits;bool signedSamples=true,littleEndian=true;};
struct NativeChannels {std::vector<std::vector<std::int32_t>> channels;int rate=0,bits=0;};
RETROMEDIALAB_MEDIAENGINE_API void validate(const Profile&);
RETROMEDIALAB_MEDIAENGINE_API Profile readProfile(const Parameters*);
RETROMEDIALAB_MEDIAENGINE_API bool tryProfile(const Parameters&,int& channels,int& rate,int& bits,int& align);
RETROMEDIALAB_MEDIAENGINE_API bool trySampleRate(const Parameters&,int&);
RETROMEDIALAB_MEDIAENGINE_API double roundEven(double);
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int32_t> extractChannel(std::span<const std::uint8_t>,int channels,int bits,int align,int channel);
RETROMEDIALAB_MEDIAENGINE_API void appendPulses(std::vector<std::int16_t>&,std::int16_t& level,std::uint16_t tStates,int count,int sampleRate,int tStatesPerSecond);
RETROMEDIALAB_MEDIAENGINE_API void appendSilence(std::vector<std::int16_t>&,int sampleRate,double seconds);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> rawDocument(std::shared_ptr<const MediaSource>,std::span<const std::uint8_t>,Profile,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API NativeChannels readChannels(const MediaDocument&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodeDocument(const MediaDocument&,Profile,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> signalDocument(std::shared_ptr<const MediaSource>,std::string format,std::span<const std::int16_t>,int sampleRate,std::stop_token={});
}
