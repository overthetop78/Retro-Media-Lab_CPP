#include "Woz.h"
#include "../../Encoding/BitPacker.h"
#include "Constants.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Woz {
namespace {
using namespace Constants;
void put(std::span<std::uint8_t> bytes,std::size_t offset,std::size_t size,std::uint32_t value){for(std::size_t i=0;i<size;++i)bytes[offset+i]=std::uint8_t(value>>(i*BitsPerByte));}
}
void appendChunk(std::vector<std::uint8_t>& output,std::string_view id,MediaFileSystems::Bytes data){
    if(id.size()!=Constants::ChunkIdLength||std::any_of(id.begin(),id.end(),[](auto c){return std::uint8_t(c)>Constants::AsciiMaximum;}))throw MediaError(DiagnosticCode::WozChunkId,{std::string(id)});
    if(data.size()>std::size_t(Constants::MaximumLength)-Constants::ChunkHeaderLength||output.size()>std::size_t(Constants::MaximumLength)-Constants::ChunkHeaderLength-data.size())throw MediaError(DiagnosticCode::WozTruncatedChunk,{std::string(id)});
    const auto offset=output.size();output.insert(output.end(),id.begin(),id.end());output.resize(output.size()+Constants::ChunkLengthSize);put(output,offset+Constants::ChunkLengthOffset,Constants::ChunkLengthSize,std::uint32_t(data.size()));output.insert(output.end(),data.begin(),data.end());
}
std::vector<std::uint8_t> build(std::span<const Flux::Bits> tracks){
    using namespace Constants;
    if(tracks.empty()||tracks.size()>TrackCount)throw MediaError(DiagnosticCode::WozTrackCount,{std::to_string(tracks.size()),std::to_string(TrackCount)});
    for(std::size_t track=0;track<tracks.size();++track)if(tracks[track].size()>Woz1MaximumBitCount)throw MediaError(DiagnosticCode::WozTrackLength,{std::to_string(track),std::to_string(tracks[track].size()),std::to_string(Woz1MaximumBitCount)});
    std::vector<std::uint8_t> output(Version1Signature.begin(),Version1Signature.end());output.insert(output.end(),Marker.begin(),Marker.end());output.resize(ChunksOffset);
    std::vector<std::uint8_t> info(InfoLength);info[InfoVersionOffset]=InfoVersion;info[InfoDiskTypeOffset]=AppleDiskType;info[InfoProtectionOffset]=Writable;info[InfoSynchronizedOffset]=Synchronized;info[InfoCleanedOffset]=Cleaned;std::copy(Creator.begin(),Creator.end(),info.begin()+InfoCreatorOffset);appendChunk(output,InfoId,info);
    std::vector<std::uint8_t> map(MapLength,MissingDescriptor);for(std::size_t track=0;track<tracks.size();++track)for(std::size_t quarter=0;quarter<MapEntriesPerTrack;++quarter)map[track*MapEntriesPerTrack+quarter]=std::uint8_t(track);appendChunk(output,TrackMapId,map);
    std::vector<std::uint8_t> entries(tracks.size()*Woz1TrackEntryLength);
    for(std::size_t track=0;track<tracks.size();++track){
        auto entry=std::span(entries).subspan(track*Woz1TrackEntryLength,Woz1TrackEntryLength);Encoding::packBits(tracks[track],entry.first(Woz1BitCountOffset),true);
        put(entry,Woz1BitCountOffset,Woz1BitCountLength,std::uint32_t(tracks[track].size()));
    }
    appendChunk(output,TracksId,entries);put(output,CrcOffset,CrcLength,crc32(MediaFileSystems::Bytes(output).subspan(ChunksOffset)));return output;
}
void write(std::span<const Flux::Bits> tracks,const std::filesystem::path& path,std::stop_token token){
    const auto output=build(tracks);writeAtomicImage(path,[&](const ImageOutput& emit,std::stop_token stop){checkCancellation(stop);emit(output);},token);
}
}
