#include "Detector.h"
#include "Constants.h"
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Detection {
namespace {
using namespace Constants;
std::optional<std::string> msa(const Context& context){
 try{const auto source=context.open();const auto header=context.read(source,0,MsaHeaderLength);if(header.size()!=MsaHeaderLength||qFromBigEndian<std::uint16_t>(header.data())!=MsaSignature)return {};
  const auto sectors=qFromBigEndian<std::uint16_t>(header.data()+MsaSectorsOffset);const auto heads=std::uint32_t(qFromBigEndian<std::uint16_t>(header.data()+MsaHeadsOffset))+1;const auto first=qFromBigEndian<std::uint16_t>(header.data()+MsaFirstCylinderOffset),last=qFromBigEndian<std::uint16_t>(header.data()+MsaLastCylinderOffset);if(sectors<MinimumSectors||sectors>MaximumSectors||heads>MaximumHeads||last<first)return {};
  const auto capacity=(std::uint64_t(last)+1)*heads*sectors*BytesPerSector/BytesPerKiB;const auto id=std::string(AtariPrefix)+std::to_string(capacity);return context.find(id)?std::optional<std::string>(id):std::nullopt;
 }catch(...){if(!probeFailure(std::current_exception()))throw;return {};}
}
std::optional<std::string_view> atx(const Context& context){try{const auto source=context.open();const auto signature=context.read(source,0,AtxHeaderLength);return signature.size()==AtxSignature.size()&&std::equal(signature.begin(),signature.end(),AtxSignature.begin())?std::optional<std::string_view>(AtariAtx):std::nullopt;}catch(...){if(!probeFailure(std::current_exception()))throw;return {};}}
}
std::optional<DetectedImageFormat> atari(const Context& context){
 using namespace Constants;std::optional<std::string> id;
 if(context.extension==St){if(const auto found=sizeFormat(StSizes,context.length))id=std::string(*found);}else if(context.extension==Msa)id=msa(context);else if(context.extension==Atr){if(const auto found=sizeFormat(AtrSizes,context.length))id=std::string(*found);}else if(context.extension==Atx){if(const auto found=atx(context))id=std::string(*found);}else return {};
 return id?context.result(*id,FormatConfidence::Certain,AtariSize):context.ambiguous(AtariUnknownSize);
}
std::optional<DetectedImageFormat> macintosh(const Context& context){
 using namespace Constants;
 if(context.extension==Image||context.extension==Dc42){const auto id=sizeFormat(MacContainerSizes,context.length);return id?context.result(id,FormatConfidence::Inferred,AppleContainer):context.ambiguous(AppleContainer);}
 if(context.extension!=Img)return {};const auto id=sizeFormat(MacRawSizes,context.length);if(!id)return {};
 try{const auto source=context.open();const auto bytes=context.read(source,MacHeaderOffset,MacSignatureLength);if(bytes.size()!=MacSignatureLength)return {};const auto signature=qFromBigEndian<std::uint16_t>(bytes.data());if(signature==MacMfsSignature||signature==MacHfsSignature)return context.result(id,FormatConfidence::Certain,AppleContainer);return {};}catch(...){if(!probeFailure(std::current_exception()))throw;return {};}
}
}
