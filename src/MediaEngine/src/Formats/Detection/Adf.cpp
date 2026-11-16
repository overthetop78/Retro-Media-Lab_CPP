#include "Detector.h"
#include <algorithm>
#include "Constants.h"
#include "../Adf/Internal.h"
#include "../../Sources/RandomAccessSources.h"
namespace RetroMediaLab::MediaEngine::Detection {
std::optional<DetectedImageFormat> adf(const Context& context){
 namespace A=Adf::Constants;using namespace Constants;
 if(context.extension==A::Adfs640Extension)return context.length==std::int64_t(A::BbcLargeCapacity)?context.result(Formats::AcornAdfs640,FormatConfidence::Certain,AcornSize):context.ambiguous(Multiple);
 if(context.length==std::int64_t(A::Capacity)&&(context.extension==AdfExtension||context.extension==Img||context.extension==A::DosPlusExtension)){
  try{const auto source=context.open();const auto header=context.read(source,0,A::FatHeaderSize);if(Adf::hasDosPlusHeader(header))return context.result(Formats::AcornDosPlus800,FormatConfidence::Certain,AcornSize);}catch(...){if(!probeFailure(std::current_exception()))throw;}
 }
 if(context.extension!=AdfExtension)return {};
 if(context.length)for(const auto& profile:Adf::amigaGeometries())if(std::int64_t(Adf::Internal::capacity(profile))==*context.length)return context.result(profile.geometry.formatId,FormatConfidence::Certain,AmigaSize);
 if(context.length&&*context.length>A::OldMapLength&&*context.length<A::HighCapacity){
  try{const auto source=context.open();const auto probe=context.read(source,0,std::size_t(std::min<std::int64_t>(A::ProbeLength,*context.length)));
   if(Adf::describePartial(probe,std::uint64_t(*context.length))){std::vector<std::uint8_t> bytes(std::size_t(*context.length));std::copy(probe.begin(),probe.end(),bytes.begin());source->readExactly(probe.size(),std::span(bytes).subspan(probe.size()),context.token);const auto partial=Adf::Internal::partial(memorySource(bytes),probe,context.token);if(partial)return context.result(partial->geometry().formatId,FormatConfidence::Certain,AcornSize);}
  }catch(...){if(!probeFailure(std::current_exception()))throw;}
 }
 if(context.length)for(const auto& profile:Adf::acornGeometries())if(profile.geometry.blockSize==A::BbcBlockSize&&std::int64_t(Adf::Internal::capacity(profile))==*context.length)return context.result(profile.geometry.formatId,FormatConfidence::Certain,AcornSize);
 if(context.length==std::int64_t(A::Capacity)||context.length==std::int64_t(A::PaddedCapacity))return context.result(Formats::AcornAdfs800,FormatConfidence::Certain,AcornSize);return {};
}
}
