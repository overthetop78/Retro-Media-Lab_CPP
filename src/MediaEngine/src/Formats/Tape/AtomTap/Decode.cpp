#include "Codec.h"
#include "Constants.h"
#include "../Uef/Constants.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
SequentialDecodeResult decode(const SequentialImage& image,std::stop_token token){
    std::vector<SequentialDecodedBlock> files;std::vector<SequentialSegment> retained,pending;std::vector<MediaFileSystems::MessageRecipe> messages;std::vector<std::uint8_t> payload;std::vector<std::int64_t> positions;std::optional<std::string> name;std::optional<std::vector<std::uint8_t>> header;int load=0,execute=0,number=0;bool valid=true;
    std::vector<const SequentialSegment*> sorted;if(image.segments())for(const auto& segment:*image.segments())sorted.push_back(&segment);std::stable_sort(sorted.begin(),sorted.end(),[](const auto* a,const auto* b){return a->fields().position<b->fields().position;});
    for(const auto* item:sorted){checkCancellation(token);const auto& segment=*item;const auto& value=segment.fields();if(value.kind==SequentialSegmentKind::Carrier||value.kind==SequentialSegmentKind::Silence)continue;const auto bytes=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token);const auto found=value.metadata.find(std::string(Uef::Constants::ChunkIdMetadataKey));const std::string_view id=found==value.metadata.end()?std::string_view{}:found->second;
        if(value.kind==SequentialSegmentKind::TapeMark&&id==BaudChunk&&bytes&&bytes->size()==WordBytes&&qFromLittleEndian<std::uint16_t>(bytes->data())==Baud)continue;
        if(value.kind!=SequentialSegmentKind::DataBlock||!bytes||bytes->size()<SerialPrefix||id!=DefinedChunk||(*bytes)[0]!=DataBits||(*bytes)[1]!=NoParity||((*bytes)[StopOffset]!=StopBits&&(*bytes)[StopOffset]!=AlternateStopBits)){retained.push_back(segment);continue;}
        pending.push_back(segment);positions.push_back(value.position);std::vector<std::uint8_t> data(bytes->begin()+SerialPrefix,bytes->end());
        if(!header){const auto endIt=data.size()>=SyncBytes?std::find(data.begin()+SyncBytes,data.end(),NameTerminator):data.end();const auto end=static_cast<std::size_t>(endIt-data.begin());if(endIt==data.end()||end<SyncBytes+1||end>SyncBytes+FileNameBytes||data.size()!=end+HeaderTail||!std::all_of(data.begin(),data.begin()+SyncBytes,[](auto v){return v==Sync;})){retained.insert(retained.end(),pending.begin(),pending.end());pending.clear();positions.clear();continue;}
            const auto blockNumber=qFromBigEndian<std::uint16_t>(data.data()+end+NumberAfterName),address=qFromBigEndian<std::uint16_t>(data.data()+end+LoadAfterName),entry=qFromBigEndian<std::uint16_t>(data.data()+end+ExecuteAfterName);std::string blockName;for(std::size_t i=SyncBytes;i<end;++i)blockName.push_back(data[i]<=MaximumAscii?static_cast<char>(data[i]):AsciiReplacement);if(!name){name=blockName;load=address;execute=entry;number=0;}valid=valid&&blockName==*name&&entry==execute&&blockNumber==number&&address==static_cast<std::uint16_t>(load+payload.size())&&(data[end+FlagsAfterName]&Continuation)==(number==0?0:Continuation);header=std::move(data);continue;
        }
        const auto terminator=header->size()-HeaderTail;const auto flags=(*header)[terminator+FlagsAfterName];const std::size_t count=(flags&ContainsData)?(*header)[terminator+CountAfterName]+CountBias:0;valid=valid&&data.size()==count+CountBias;
        if(data.size()!=count+CountBias){retained.insert(retained.end(),pending.begin(),pending.end());pending.clear();positions.clear();header.reset();name.reset();payload.clear();valid=true;messages.push_back({std::string(DiagnosticKeys::AtomTapTruncated),{},{}});continue;}
        unsigned checksum{};for(auto byte:*header)checksum+=byte;for(std::size_t i=0;i<count;++i)checksum+=data[i];valid=valid&&static_cast<std::uint8_t>(checksum)==data[count];if(count>MaximumBuffer-payload.size())throw MediaError(DiagnosticCode::AtomTapLimit);payload.insert(payload.end(),data.begin(),data.begin()+count);header.reset();++number;if(flags&MoreBlocks)continue;
        std::map<std::string,std::string> metadata{{std::string(FileNameKey),*name},{std::string(BlockNumberKey),std::string(FirstBlock)},{std::string(LoadKey),QString::number(load,HexRadix).rightJustified(HexWidth,DigitZero).toStdString()},{std::string(ExecuteKey),QString::number(execute,HexRadix).rightJustified(HexWidth,DigitZero).toStdString()}};files.emplace_back(files.size(),std::move(payload),std::move(positions),valid,std::move(metadata));pending.clear();positions.clear();payload.clear();name.reset();valid=true;
    }
    if(!pending.empty()){retained.insert(retained.end(),pending.begin(),pending.end());messages.push_back({std::string(DiagnosticKeys::AtomTapIncomplete),{},{}});}const auto count=std::count_if(files.begin(),files.end(),[](const auto& file){return file.integrityValid()==true;});if(std::size_t(count)!=files.size())messages.push_back({std::string(DiagnosticKeys::AtomTapIntegrity),{},{}});const auto confidence=files.empty()?0:double(count)/files.size();return SequentialDecodeResult(std::string(DecoderId),confidence,std::move(files),std::move(retained),{},std::move(messages));
}
}
