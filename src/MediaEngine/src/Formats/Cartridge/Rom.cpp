#include "Internal.h"
namespace RetroMediaLab::MediaEngine::Cartridge {
using namespace Constants;using namespace Internal;
namespace {
bool corpus(std::int64_t length){return std::find(RomCorpusLengths.begin(),RomCorpusLengths.end(),length)!=RomCorpusLengths.end();}
bool amsdos(M::Bytes header){if(header.size()<OptionalRomHeaderLength)return false;std::uint32_t sum=0;for(int index=0;index<=RomChecksumLast;++index)sum=(sum+header[index])&RomChecksumMask;return sum==(std::uint32_t(header[RomChecksumLow])|(std::uint32_t(header[RomChecksumHigh])<<std::numeric_limits<std::uint8_t>::digits));}
bool dandanator(M::Bytes bytes){for(std::size_t index=0;index<=bytes.size()&&CommandLength<=bytes.size()-index;++index){bool prefix=true;for(int repeat=0;repeat<CommandRepeat;++repeat)if(bytes[index+repeat]!=CommandByte)prefix=false;const auto command=bytes[index+CommandRepeat];if(prefix&&(command==CommandFirst||command==CommandSecond||command==CommandThird))return true;}return false;}
}
const ReaderCapabilities& RomReader::capabilities()const{static const ReaderCapabilities value{{std::string(Formats::AmstradRom)},{std::string(RomExtension),std::string(BinExtension)},{},{},{MediaKind::Cartridge},{RepresentationKind::Blocks}};return value;}
std::future<bool> RomReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    M::checkCancellation(token);if((!sameIdentifier(context.extension(),RomExtension)&&!sameIdentifier(context.extension(),BinExtension))||!corpus(context.length()))return false;
    if(context.requestedFormatId())return requested(context,Formats::AmstradRom);if(!sameIdentifier(context.extension(),RomExtension))return false;
    if(context.length()==BankLength+OptionalRomHeaderLength)return amsdos(context.readHeaderAsync(OptionalRomHeaderLength,token).get());
    if(context.length()==std::int64_t(MaximumBankCount)*BankLength)return dandanator(context.readBytesAsync(token).get());return false;
});}
std::future<std::shared_ptr<const MediaDocument>> RomReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    M::checkCancellation(token);if(!corpus(context.length()))throw MediaError(DiagnosticCode::RomCorpus);
    const auto headerLength=context.length()%BankLength==OptionalRomHeaderLength?OptionalRomHeaderLength:0;const auto dataLength=context.length()-headerLength;if(dataLength<=0)throw MediaError(DiagnosticCode::RomNoData);const auto count=integer((dataLength+BankLength-1)/BankLength);if(count>MaximumBankCount)throw MediaError(DiagnosticCode::RomBankLimit);
    const auto source=context.dataSource();std::vector<DataRange> ranges;std::map<std::string,std::string> metadata={{std::string(SystemIdKey),std::string(AmstradSystem)},{std::string(RoleKey),std::string(RomRole)},{std::string(BankSizeKey),std::to_string(BankLength)},{std::string(BankCountKey),std::to_string(count)},{std::string(SourceHeaderLengthKey),std::to_string(headerLength)}};
    for(std::int32_t bank=0;bank<count;++bank){const auto address=std::int64_t(bank)*BankLength,length=std::min(std::int64_t(BankLength),dataLength-address);ranges.emplace_back(address,length,DataRangeKind::Stored,source,headerLength+address);metadata[bankKey(bank,BankLengthSuffix)]=std::to_string(length);}
    const auto capacity=std::int64_t(count)*BankLength;std::vector<M::MessageRecipe> messages;if(dataLength%BankLength)messages.push_back(message(DiagnosticCode::RomIncomplete));
    return std::make_shared<MediaDocument>(context.source(),std::string(Formats::AmstradRom),MediaKind::Cartridge,std::make_shared<BlockImage>(capacity,1,std::move(ranges)),std::vector<M::VolumeDescriptor>{volume(capacity,RomVolumeKey)},std::move(messages),std::move(metadata));
});}
}
