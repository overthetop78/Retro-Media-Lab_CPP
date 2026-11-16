#include <RetroMediaLab/MediaEngine/PhysicalMedia.h>
#include <RetroMediaLab/MediaEngine/PhysicalMediaIds.h>
#include <RetroMediaLab/MediaEngine/TapePreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include "../Primitives/Identifiers.h"
#include <bit>
#include <charconv>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace P=PhysicalMediaIds;namespace C=PhysicalDetails;
namespace {
[[noreturn]] void invalid(std::string_view field){throw MediaError(DiagnosticCode::PhysicalContract,{std::string(field)});}
std::string_view metadata(const PhysicalDataUnit& unit,std::string_view key){auto i=unit.metadata().find(std::string(key));if(i==unit.metadata().end())throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(key)});return i->second;}
int sampleRate(const PhysicalDataUnit& unit){auto text=metadata(unit,P::SampleRate);if(text.empty()||text.front()<C::FirstDigit||text.front()>C::LastDigit)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::SampleRate)});int result=0;auto parsed=std::from_chars(text.data(),text.data()+text.size(),result);if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||result<=0)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::SampleRate)});return result;}
}
PhysicalDataUnit createPhysicalTapeUnit(const MediaTapeSignal& signal,std::int64_t firstSample){
 if(firstSample<0)invalid(C::PositionField);if(signal.sampleRate()<=0||signal.samples().empty()||signal.samples().size()>std::size_t(std::numeric_limits<int>::max())/C::SampleBytes)invalid(C::SamplesField);
 std::vector<std::uint8_t> bytes;bytes.reserve(signal.samples().size()*C::SampleBytes);for(auto sample:signal.samples()){auto value=std::bit_cast<std::uint16_t>(sample);bytes.push_back(std::uint8_t(value&C::ByteMask));bytes.push_back(std::uint8_t(value>>C::ByteBits));}
 return {firstSample,std::move(bytes),{{std::string(P::Encoding),std::string(P::TapePcm)},{std::string(P::SampleRate),std::to_string(signal.sampleRate())}}};
}
MediaTapeSignal readPhysicalTapeSignal(std::span<const PhysicalDataUnit> units){
 if(units.empty())invalid(C::UnitsField);std::size_t count=0;int rate=0;
 for(const auto& unit:units){if(metadata(unit,P::Encoding)!=P::TapePcm)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::Encoding)});const auto current=sampleRate(unit);if(rate&&current!=rate)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::SampleRate)});rate=current;
 if(unit.position()!=std::int64_t(count)||unit.data().empty()||unit.data().size()%C::SampleBytes)invalid(C::UnitsField);const auto added=unit.data().size()/C::SampleBytes;if(added>std::size_t(std::numeric_limits<int>::max())-count)invalid(C::SamplesField);count+=added;}
 std::vector<std::int16_t> samples;samples.reserve(count);for(const auto& unit:units)for(std::size_t i=0;i<unit.data().size();i+=C::SampleBytes){auto value=std::uint16_t(std::uint16_t(unit.data()[i])|(std::uint16_t(unit.data()[i+1])<<C::ByteBits));samples.push_back(std::bit_cast<std::int16_t>(value));}return {std::move(samples),rate};
}
MediaTapeSignal createAcquiredTapeSignal(const PhysicalAcquisitionResult& result){if(result.mediaKind()!=MediaKind::Tape||result.representationKind()!=RepresentationKind::Sequential)invalid(C::KindField);return readPhysicalTapeSignal(result.dataUnits());}
PhysicalWritePlan createTapeWritePlan(const MediaTapeSignal& signal){return {MediaKind::Tape,RepresentationKind::Sequential,{createPhysicalTapeUnit(signal)},{0}};}
std::future<PhysicalWritePlan> createTapeWritePlanAsync(std::filesystem::path path,PhysicalMetadata parameters,std::stop_token token){return std::async(std::launch::async,[path=std::move(path),parameters=std::move(parameters),token]{checkCancellation(token);const auto encoded=path.u8string();if(Primitives::isBlankIdentifier(std::string(encoded.begin(),encoded.end())))invalid(C::PathField);auto signal=readTapeAsync(path,token,{},parameters).get();checkCancellation(token);return createTapeWritePlan(signal);});}
}
