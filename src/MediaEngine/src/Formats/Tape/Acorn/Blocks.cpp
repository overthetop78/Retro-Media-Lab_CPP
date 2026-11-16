#include "Blocks.h"
#include "Constants.h"
#include "../Uef/Constants.h"
#include "../../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <charconv>
#include <limits>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AcornTape {
namespace {
using namespace Constants;using namespace Uef::Constants;
struct Header {std::string_view name;std::uint32_t load{},execute{},next{};std::uint16_t number{};std::uint8_t flags{};};
template<class T>bool number(const std::map<std::string,std::string>& metadata,std::string_view key,T& value,int radix){
    const auto found=metadata.find(std::string(key));if(found==metadata.end())return false;
    const auto text=radix==RadixHex?QString::fromUtf8(found->second).trimmed().toStdString():found->second;
    if(text.empty())return false;std::uint64_t result{};const auto parsed=std::from_chars(text.data(),text.data()+text.size(),result,radix);
    if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||result>std::numeric_limits<T>::max())return false;
    value=static_cast<T>(result);return true;
}
std::optional<Header> describe(const SequentialDecodedBlock& block){
    Header result;const auto& metadata=block.metadata();const auto name=metadata.find(std::string(FileNameMetadataKey));
    if(block.data().size()>AcornMaximumDataLength||name==metadata.end()||name->second.empty()||name->second.size()>AcornMaximumFileNameLength
        ||std::any_of(name->second.begin(),name->second.end(),[](unsigned char value){return value==0||value>MaximumAscii;})
        ||!number(metadata,LoadAddressMetadataKey,result.load,RadixHex)||!number(metadata,ExecutionAddressMetadataKey,result.execute,RadixHex)
        ||!number(metadata,BlockNumberMetadataKey,result.number,RadixDecimal)||!number(metadata,FlagsMetadataKey,result.flags,RadixHex)
        ||!number(metadata,NextAddressMetadataKey,result.next,RadixHex))return {};
    result.name=name->second;return result;
}
std::string hex(std::uint32_t value,int width){return QString::number(value,RadixHex).rightJustified(width,ZeroDigit).toStdString();}
}
bool canBuildBlock(const SequentialDecodedBlock& block){return describe(block).has_value();}
std::vector<std::uint8_t> buildBlock(const SequentialDecodedBlock& block,std::stop_token token){
    checkCancellation(token);const auto header=describe(block);if(!header)throw MediaError(DiagnosticCode::AcornTapeEncode);
    const auto headerLength=header->name.size()+1+FixedHeaderLength;
    std::vector<std::uint8_t> bytes(1+headerLength+AcornChecksumLength+block.data().size()+AcornChecksumLength);bytes[0]=AcornBlockSync;
    std::copy(header->name.begin(),header->name.end(),bytes.begin()+1);const auto fixed=1+header->name.size()+1;
    qToLittleEndian(header->load,bytes.data()+fixed+LoadOffset);qToLittleEndian(header->execute,bytes.data()+fixed+ExecuteOffset);
    qToLittleEndian(header->number,bytes.data()+fixed+NumberOffset);qToLittleEndian(static_cast<std::uint16_t>(block.data().size()),bytes.data()+fixed+DataLengthOffset);
    bytes[fixed+FlagsOffset]=header->flags;qToLittleEndian(header->next,bytes.data()+fixed+NextOffset);
    qToBigEndian(Crc16::compute(std::span<const std::uint8_t>(bytes).subspan(1,headerLength),AcornChecksumPolynomial,AcornChecksumInitialValue,token),bytes.data()+1+headerLength);
    const auto payload=1+headerLength+AcornChecksumLength;std::copy(block.data().begin(),block.data().end(),bytes.begin()+payload);
    qToBigEndian(Crc16::compute(block.data(),AcornChecksumPolynomial,AcornChecksumInitialValue,token),bytes.data()+payload+block.data().size());return bytes;
}
std::vector<SequentialDecodedBlock> decodeBlocks(std::span<const std::uint8_t> bytes,const std::vector<std::int64_t>& positions,std::string_view sourceKind,std::size_t firstPosition,std::span<const int> byteSamples,std::stop_token token){
    std::vector<SequentialDecodedBlock> blocks;
    for(std::size_t offset=0;offset<bytes.size();++offset){
        checkCancellation(token);if(bytes[offset]!=AcornBlockSync)continue;const auto nameStart=offset+1;auto nameEnd=nameStart;
        while(nameEnd<bytes.size()&&nameEnd-nameStart<=AcornMaximumFileNameLength&&bytes[nameEnd]!=0)++nameEnd;
        if(nameEnd>=bytes.size()||bytes[nameEnd]!=0||nameEnd-nameStart>AcornMaximumFileNameLength)continue;
        const auto fixed=nameEnd+1;if(bytes.size()-fixed<FixedHeaderLength+AcornChecksumLength)continue;
        const auto headerEnd=fixed+FixedHeaderLength;const auto length=qFromLittleEndian<std::uint16_t>(bytes.data()+fixed+DataLengthOffset);
        const auto dataStart=headerEnd+AcornChecksumLength;if(bytes.size()-dataStart<std::size_t(length)+AcornChecksumLength)continue;
        const auto data=bytes.subspan(dataStart,length);const auto end=dataStart+length+AcornChecksumLength;
        const bool headerValid=Crc16::compute(bytes.subspan(nameStart,headerEnd-nameStart),AcornChecksumPolynomial,AcornChecksumInitialValue,token)==qFromBigEndian<std::uint16_t>(bytes.data()+headerEnd);
        const bool dataValid=Crc16::compute(data,AcornChecksumPolynomial,AcornChecksumInitialValue,token)==qFromBigEndian<std::uint16_t>(bytes.data()+dataStart+length);
        std::string name;for(auto value:bytes.subspan(nameStart,nameEnd-nameStart))name.push_back(value<=MaximumAscii?static_cast<char>(value):AsciiReplacement);
        std::map<std::string,std::string> metadata{{std::string(SourceKindMetadataKey),std::string(sourceKind)},{std::string(FileNameMetadataKey),std::move(name)},
            {std::string(LoadAddressMetadataKey),hex(qFromLittleEndian<std::uint32_t>(bytes.data()+fixed+LoadOffset),AddressDigits)},
            {std::string(ExecutionAddressMetadataKey),hex(qFromLittleEndian<std::uint32_t>(bytes.data()+fixed+ExecuteOffset),AddressDigits)},
            {std::string(BlockNumberMetadataKey),std::to_string(qFromLittleEndian<std::uint16_t>(bytes.data()+fixed+NumberOffset))},
            {std::string(FlagsMetadataKey),hex(bytes[fixed+FlagsOffset],FlagDigits)},
            {std::string(NextAddressMetadataKey),hex(qFromLittleEndian<std::uint32_t>(bytes.data()+fixed+NextOffset),AddressDigits)},
            {std::string(ByteOffsetKey),std::to_string(offset)}};
        if(offset<byteSamples.size())metadata[std::string(SamplePositionKey)]=std::to_string(byteSamples[offset]);
        blocks.emplace_back(firstPosition+blocks.size(),std::vector<std::uint8_t>(data.begin(),data.end()),positions,headerValid&&dataValid,std::move(metadata));offset=end-1;
    }
    return blocks;
}
}
