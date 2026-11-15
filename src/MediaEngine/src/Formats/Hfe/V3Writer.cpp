#include "Hfe.h"
#include "Constants.h"
#include "../../Models/Flux/Conversion.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaEngine::Hfe {
namespace {
using namespace Constants;
class TrackWriter {
 std::vector<std::uint8_t> bytes_{Index};std::uint32_t buffer_=0,count_=0;
 void append(std::uint8_t value){if(bytes_.size()>=MaxSide)throw MediaError(DiagnosticCode::HfeLimit);bytes_.push_back(value);}
 static std::uint8_t reverse(std::uint8_t value){std::uint8_t result=0;for(std::uint32_t i=0;i<BitsPerByte;++i)result|=std::uint8_t(((value>>i)&1)<<(BitsPerByte-1-i));return result;}
 void rate(std::uint8_t divider){append(Rate);append(reverse(divider));}
 void bit(bool transition){if(transition)buffer_|=1<<count_;if(++count_!=BitsPerByte)return;if((buffer_&OpcodeMask)==OpcodeMask){append(Skip);append(reverse(1));append(std::uint8_t((buffer_&SevenBitsMask)<<1));buffer_>>=BitsPerByte-1;count_=1;}else{append(std::uint8_t(buffer_));buffer_=0;count_=0;}}
 void flush(){if(!count_)return;const auto skip=BitsPerByte-count_;append(Skip);append(reverse(std::uint8_t(skip)));append(std::uint8_t(buffer_<<skip));buffer_=0;count_=0;}
 void interval(std::uint32_t ns,bool transition){auto clocks=std::uint64_t(ns)*ClockMegahertz;if(clocks%NanosecondsPerMicrosecond)throw MediaError(DiagnosticCode::HfeTiming);clocks/=NanosecondsPerMicrosecond;const auto cells=clocks/NominalDivider;const auto remainder=std::uint8_t(clocks%NominalDivider);for(std::uint64_t cell=0;cell<cells;++cell)bit(transition&&!remainder&&cell+1==cells);if(remainder){flush();rate(remainder);bit(transition);flush();rate(NominalDivider);}}
public:
 std::vector<std::uint8_t> encode(const Flux::ProtectedTrack& track){if(std::any_of(track.features().begin(),track.features().end(),[](const auto& f){return f.kind==Flux::TrackFeatureKind::WeakRegion;}))throw MediaError(DiagnosticCode::HfeWeak);const auto flux=Flux::readFlux(track);if(flux.size()!=1)throw MediaError(DiagnosticCode::HfeRevolutions);const auto& revolution=flux.front();rate(NominalDivider);std::uint64_t position=0;for(auto value:revolution.intervals){position+=value;if(position>revolution.indexTime)throw MediaError(DiagnosticCode::HfeFluxBounds);interval(value,true);}const auto tail=revolution.indexTime-position;if(tail)interval(std::uint32_t(tail),false);flush();return std::move(bytes_);}
};
void word(std::vector<std::uint8_t>& bytes,std::size_t offset,std::size_t value){if(value>MaxField)throw MediaError(DiagnosticCode::HfeLimit);bytes[offset]=std::uint8_t(value);bytes[offset+1]=std::uint8_t(value>>BitsPerByte);}
}
std::vector<std::uint8_t> buildV3(const Flux::ProtectedTrackImage& image,const Metadata& metadata){using namespace Constants;std::int32_t cylinders=0,heads=0;std::map<std::pair<std::int32_t,std::int32_t>,std::vector<std::uint8_t>> encoded;for(const auto&t:image.tracks()){if(t.cylinder()==std::numeric_limits<std::int32_t>::max()||t.head()==std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::HfeLimit);cylinders=std::max(cylinders,t.cylinder()+1);heads=std::max(heads,t.head()+1);}if(cylinders>std::int32_t(MaxCylinder)||heads>std::int32_t(MaxHeads))throw MediaError(DiagnosticCode::HfeLimit);for(const auto& t:image.tracks())encoded.emplace(std::pair{t.cylinder(),t.head()},TrackWriter{}.encode(t));const auto blocks=(cylinders*EntrySize+BlockSize-1)/BlockSize;std::vector<std::uint8_t> table(blocks*BlockSize),body;std::size_t block=blocks+1;
 for(std::int32_t cylinder=0;cylinder<cylinders;++cylinder){std::vector<std::vector<std::uint8_t>> sides;std::size_t length=0;for(std::int32_t head=0;head<heads;++head){const auto found=encoded.find({cylinder,head});if(found==encoded.end())throw MediaError(DiagnosticCode::HfeMissingTrack,{std::to_string(cylinder)});sides.push_back(found->second);length=std::max(length,sides.back().size());}word(table,cylinder*EntrySize,block);word(table,cylinder*EntrySize+LengthOffset,length*MaxHeads);appendTrackData(body,sides,length,Nop);block+=(length+SideChunk-1)/SideChunk;}
 Image i;i.cylinders=cylinders;i.heads=heads;i.encoding=resolveEncoding(metadata);i.bitRate=NominalBitRate;i.writeProtected=image.writeProtected();auto result=header(i);std::copy(Signature3.begin(),Signature3.end(),result.begin());result.insert(result.end(),table.begin(),table.end());result.insert(result.end(),body.begin(),body.end());return result;
}
}
