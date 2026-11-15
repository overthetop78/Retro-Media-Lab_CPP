#include "Nib.h"
#include "Constants.h"
#include "../../Encoding/BitPacker.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
namespace RetroMediaLab::MediaEngine::Nib {
std::vector<std::uint8_t> build(std::span<const Flux::Bits> tracks){
    if(tracks.empty())throw MediaError(DiagnosticCode::NibTrackCount,{std::to_string(tracks.size())});
    for(std::size_t track=0;track<tracks.size();++track)if(tracks[track].size()>Constants::MaximumTrackBitCount)throw MediaError(DiagnosticCode::NibTrackLength,{std::to_string(track),std::to_string(tracks[track].size()),std::to_string(Constants::MaximumTrackBitCount)});
    if(tracks.size()>std::size_t(std::numeric_limits<std::int32_t>::max())/Constants::TrackLengthBytes)throw MediaError(DiagnosticCode::BitPackingRange);
    std::vector<std::uint8_t> bytes(tracks.size()*Constants::TrackLengthBytes);
    for(std::size_t track=0;track<tracks.size();++track)Encoding::packBits(tracks[track],std::span(bytes).subspan(track*Constants::TrackLengthBytes,Constants::TrackLengthBytes),true);
    return bytes;
}
void write(std::span<const Flux::Bits> tracks,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(tracks);writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(bytes);},token);}
}
