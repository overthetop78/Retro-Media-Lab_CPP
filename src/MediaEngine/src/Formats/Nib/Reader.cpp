#include "Nib.h"
#include "Constants.h"
#include "../Apple/DecodedTracks.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Nib {
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes data,std::stop_token token){
    if(data.empty()||data.size()%Constants::TrackLengthBytes)throw MediaError(DiagnosticCode::RawInvalidLength,{std::to_string(data.size()),std::to_string(Constants::TrackLengthBytes)});
    std::vector<Apple::DecodedTrack> standard,rwts;std::size_t credible=0;
    for(std::size_t track=0;track<data.size()/Constants::TrackLengthBytes;++track){
        checkCancellation(token);Flux::Bits bits(Constants::MaximumTrackBitCount);
        for(std::size_t byte=0;byte<Constants::TrackLengthBytes;++byte)for(std::size_t bit=0;bit<Constants::BitsPerByte;++bit)
            bits[byte*Constants::BitsPerByte+bit]=std::uint8_t((data[track*Constants::TrackLengthBytes+byte]>>(Constants::BitsPerByte-1-bit))&1);
        auto result=Apple::decodeTrack(bits,std::int32_t(track),token);if(!result.rwts18.empty())++credible;
        standard.push_back({std::int32_t(track),std::move(result.standard)});rwts.push_back({std::int32_t(track),std::move(result.rwts18)});
    }
    return credible>=Constants::MinimumCredibleRwts18TrackCount?Apple::buildRwts18(rwts):Apple::buildStandard(standard);
}
}
