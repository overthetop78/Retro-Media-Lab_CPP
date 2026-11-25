#include "Providers.h"
#include "Constants.h"
#include "../Models/Blocks/BlockImage.h"
#include "../Formats/Cartridge/ConsoleConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
#include <charconv>
#include <limits>
#include <set>
namespace RetroMediaLab::MediaEngine::Visualization {
namespace {
std::int64_t add(std::int64_t a,std::int64_t b){if(a<0||b<0||a>std::numeric_limits<std::int64_t>::max()-b)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::BlockOverflow);return a+b;}
std::int64_t multiply(std::int64_t a,std::int64_t b){if(a<0||b<0||(a&&b>std::numeric_limits<std::int64_t>::max()/a))throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::BlockOverflow);return a*b;}
std::int64_t visualLength(const MediaDocument& document,const BlockImage& image){
 if(document.mediaKind()!=MediaKind::Cartridge)return image.capacity();const auto found=document.metadata().find(std::string(Cartridge::ConsoleConstants::StoredLengthKey));if(found==document.metadata().end()||found->second.empty())return image.capacity();const auto& text=found->second;
 if(std::any_of(text.begin(),text.end(),[](char value){return value<Constants::FirstDigit||value>Constants::LastDigit;}))return image.capacity();std::int64_t length=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),length);return result.ec==std::errc{}&&result.ptr==text.data()+text.size()&&length>0&&length<=image.capacity()?length:image.capacity();
}
class Blocks final:public Provider {
 inline static constexpr std::array Kinds{RepresentationKind::Blocks};
public:
 std::span<const RepresentationKind> kinds()const noexcept override{return Kinds;}
 bool canProvide(const MediaDocument& document)const override{return dynamic_cast<const BlockImage*>(&document.representation())!=nullptr;}
 VisualizationDescriptor describe(const MediaDocument& document,std::stop_token token)const override{
  const auto& image=representation<BlockImage>(document);VisualizationDescriptorFields result{RepresentationKind::Blocks,{},VisualizationProgressUnit::BlockRange,VisualizationDirection::Ascending,{},{}};
  const auto length=visualLength(document,image);std::set<std::int64_t> boundaries{0,length};
  if(image.geometry()){
   const auto& geometry=*image.geometry();for(std::int32_t head=0;head<geometry.heads();++head)result.surfaces.push_back(head);
   const auto trackBytes=multiply(geometry.sectorsPerTrack(),image.logicalBlockSize());
   for(std::int64_t start=0;start<geometry.capacity();start=add(start,trackBytes)){checkCancellation(token);boundaries.insert(start);}boundaries.insert(geometry.capacity());
  }
  for(const auto& range:image.ranges()){checkCancellation(token);if(range.address()>=length)continue;boundaries.insert(range.address());boundaries.insert(std::min(length,add(range.address(),range.length())));}
  for(const auto& volume:document.volumes()){checkCancellation(token);const auto start=std::int64_t(volume.start());if(start>=length)continue;boundaries.insert(start);boundaries.insert(std::min(length,add(start,std::int64_t(volume.length()))));}
  if(result.surfaces.empty())result.surfaces.push_back(0);if(!boundaries.empty()){auto previous=boundaries.begin();for(auto point=std::next(previous);point!=boundaries.end();++point){checkCancellation(token);const auto distance=*point-*previous;result.elements.emplace_back(VisualizationElementFields{*previous/image.logicalBlockSize(),image.geometry() && *previous<image.geometry()->capacity()?static_cast<std::int32_t>((*previous/image.logicalBlockSize()/image.geometry()->sectorsPerTrack())%image.geometry()->heads()):0,(distance-1)/image.logicalBlockSize()+1});previous=point;}}
  return VisualizationDescriptor(std::move(result));
 }
};
}
std::shared_ptr<const Provider> blockProvider(){return std::make_shared<Blocks>();}
bool readBlockElement(const MediaDocument& document,const VisualizationElement& element,std::stop_token token){
 const auto image=dynamic_cast<const BlockImage*>(&document.representation());if(!image)throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Constants::BlockImage)});const auto& fields=element.fields();const auto address=multiply(fields.position,image->logicalBlockSize());const auto limit = visualLength(document, *image);
 if(address >= limit) throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Constants::Length)});
 const auto length = std::min(multiply(fields.length.value_or(1),image->logicalBlockSize()), limit-address);
 std::vector<std::uint8_t> buffer(std::size_t(std::min<std::int64_t>(Constants::ReadBufferSize,length)));
 try{for(std::int64_t offset=0;offset<length;){checkCancellation(token);const auto count=std::size_t(std::min<std::int64_t>(buffer.size(),length-offset));image->readExactly(address+offset,std::span(buffer).first(count),token);offset+=std::int64_t(count);}return true;}
 catch(const MediaFileSystems::MediaError& error){if(error.kind()==MediaFileSystems::ErrorKind::Io||error.kind()==MediaFileSystems::ErrorKind::InvalidData)return false;throw;}
}
}
