#include "Hfe.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Hfe {
std::vector<std::uint8_t> pack(Flux::BitView bits){if(bits.size()>Constants::MaxLength-Constants::BitMask)throw MediaError(DiagnosticCode::HfeLimit);std::vector<std::uint8_t> result((bits.size()+Constants::BitMask)/Constants::BitsPerByte);for(std::size_t i=0;i<bits.size();++i)if(bits[i])result[i/Constants::BitsPerByte]|=std::uint8_t(1<<(i%Constants::BitsPerByte));return result;}
Flux::Bits unpack(MediaFileSystems::Bytes bytes){if(bytes.size()>Constants::MaxLength/Constants::BitsPerByte)throw MediaError(DiagnosticCode::HfeLimit);Flux::Bits bits(bytes.size()*Constants::BitsPerByte);for(std::size_t i=0;i<bits.size();++i)bits[i]=std::uint8_t((bytes[i/Constants::BitsPerByte]>>(i%Constants::BitsPerByte))&1);return bits;}
Track uniformTrack(std::int32_t cylinder,std::int32_t head,Flux::Bits bits,std::uint32_t ticks){if(std::uint64_t(bits.size())*ticks>Constants::MaxTicks)throw MediaError(DiagnosticCode::HfeLimit);Track t;t.cylinder=cylinder;t.head=head;t.bitCellTicks=ticks;t.bits=std::move(bits);t.timing.push_back({0,std::int64_t(t.bits.size()),double(ticks)*Constants::TickNanoseconds});t.revolution=Flux::createRevolution(t.bits,ticks,std::uint32_t(t.bits.size()*ticks));return t;}
Metadata metadata(const Image& image){return {{std::string(Constants::RevisionKey),std::to_string(image.revision)},{std::string(Constants::EncodingKey),std::to_string(image.encoding)},{std::string(Constants::BitRateKey),std::to_string(image.bitRate)}};}
Flux::ProtectedTrackImage protectedImage(const Image& image){std::vector<Flux::ProtectedTrack> tracks;for(const auto& t:image.tracks)tracks.emplace_back(t.cylinder,t.head,t.bits,t.timing,std::vector<Decoding::Structure>{},t.features,std::vector<Flux::TrackFluxRevolution>{{Constants::TickNanoseconds,t.revolution}});return {std::move(tracks),image.writeProtected};}
}
