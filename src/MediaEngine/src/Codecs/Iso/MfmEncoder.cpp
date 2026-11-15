#include "MfmInternal.h"
#include "../../Encoding/Bits.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoMfm {
bool Encoder::canEncodeEmptyTrack(const Encoding::Request& request)const{return Internal::physical(request);}
Flux::Revolution Encoder::encodeRevolution(const Encoding::Request& request,Flux::BitView bits,std::stop_token token)const{return Flux::createRevolution(bits,request.cellTicks,request.indexTicks,!Internal::physical(request),token);}
Flux::Bits Encoder::encodeBits(const Encoding::Request& request,std::stop_token token)const{
    using namespace Constants;
    if(!Internal::physical(request))return encodeIsoBits(request,Internal::validateAddress,Internal::sizeCode,[&](auto& bits,const auto& r,const auto& sector,auto code){Internal::address(bits,r,sector,code,HeaderGapBits,token);},[&](auto& bits,const auto& sector){Internal::data(bits,sector,DataGapBits,token);},token);
    const auto capacity=std::uint64_t(request.indexTicks)/request.cellTicks;
    if(capacity>Encoding::Constants::MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
    const auto requireComplete=Internal::complete(request);std::size_t completed=0;Flux::Bits bits;
    const auto capacityError=[&]{throw MediaError(DiagnosticCode::TrackPhysicalCapacity,{std::string(Id),std::to_string(request.cylinder),std::to_string(request.head),std::to_string(capacity)});};
    Internal::run(bits,GapFill,InitialGapBytes,token);
    for(const auto& sector:request.sectors){
        checkCancellation(token);Internal::validateAddress(Encoding::Constants::NumberField,sector.number);const auto code=Internal::sizeCode(sector);
        Internal::run(bits,SyncFill,SyncZeroBytes,token);Internal::address(bits,request,sector,code,0,token);
        Internal::run(bits,GapFill,InterFieldGapBytes,token);Internal::run(bits,SyncFill,SyncZeroBytes,token);Internal::data(bits,sector,0,token);++completed;
        if(requireComplete&&bits.size()>capacity)capacityError();
        Internal::run(bits,GapFill,InterSectorGapBytes,token);
        if(bits.size()>=capacity){if(requireComplete&&completed<request.sectors.size())capacityError();break;}
    }
    const auto remaining=capacity>bits.size()?capacity-bits.size():0;
    Internal::run(bits,GapFill,std::int32_t((remaining+EncodedByteBits-1)/EncodedByteBits),token);
    if(bits.size()>capacity)bits.resize(std::size_t(capacity));return bits;
}
}
