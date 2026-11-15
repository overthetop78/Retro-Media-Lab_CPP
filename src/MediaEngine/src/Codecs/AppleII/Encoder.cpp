#include "Gcr.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AppleII {
namespace {
void validate(std::string_view field,std::int32_t value){if(value<0||value>Constants::MaximumAddressValue)throw MediaError(DiagnosticCode::AppleIIAddressRange,{std::string(field),std::to_string(value),std::to_string(Constants::MaximumAddressValue)});}
}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const {
 using namespace Constants;
 const auto volume=attribute(request.attributes,std::string(VolumeAttribute),DefaultVolume);
 validate(VolumeField,volume);validate(Encoding::Constants::CylinderField,request.cylinder);
 const auto actualCount=std::int64_t(request.sectors.size());auto count=actualCount;bool explicitFormat=false;
 if(request.attributes){const auto it=request.attributes->find(std::string(SectorsPerTrackAttribute));if(it!=request.attributes->end()){explicitFormat=true;count=it->second;}}
 if((count!=FiveAndThreeSectorsPerTrack&&count!=SixAndTwoSectorsPerTrack)||(explicitFormat&&(request.sectors.size()==FiveAndThreeSectorsPerTrack||request.sectors.size()==SixAndTwoSectorsPerTrack)&&actualCount!=count))throw MediaError(DiagnosticCode::AppleIISectorsPerTrack,{std::to_string((count==FiveAndThreeSectorsPerTrack||count==SixAndTwoSectorsPerTrack)?actualCount:count)});
 const bool five=count==FiveAndThreeSectorsPerTrack;Flux::Bits bits;
 for(const auto& sector:request.sectors){
  checkCancellation(token);if(sector.data.size()!=SectorSize)throw MediaError(DiagnosticCode::AppleIISectorSize,{std::to_string(sector.data.size())});validate(SectorNumberField,sector.number);
  Encoding::gap(bits,LeadingGapBitCount,true);Encoding::raw(bits,five?FiveAndThreeAddressPrologueBytes:SixAndTwoAddressPrologueBytes);
  Encoding::raw(bits,encodeAddress(std::uint8_t(volume),std::uint8_t(request.cylinder),std::uint8_t(sector.number)));Encoding::raw(bits,AddressToDataSeparatorBytes);
  Encoding::raw(bits,five?encodeFiveAndThree(sector.data,token):encodeSixAndTwo(sector.data,token));Encoding::raw(bits,EpilogueBytes);Encoding::gap(bits,TrailingGapBitCount);
 }
 return bits;
}
}
