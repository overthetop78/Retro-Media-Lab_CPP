#include "Internal.h"
#include <map>
namespace RetroMediaLab::MediaEngine::Cartridge {
using namespace Constants;using namespace Internal;
const ReaderCapabilities& CprReader::capabilities()const{static const ReaderCapabilities value{{std::string(Formats::AmstradCpr)},{std::string(CprExtension)},{std::vector<std::uint8_t>(Riff.begin(),Riff.end())},{},{MediaKind::Cartridge},{RepresentationKind::Blocks}};return value;}
std::future<bool> CprReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(!requested(context,Formats::AmstradCpr)||context.length()<CprHeaderLength||!sameIdentifier(context.extension(),CprExtension))return false;const auto header=context.readHeaderAsync(CprHeaderLength,token).get();return starts(header,Riff)&&starts(M::Bytes(header).subspan(FormOffset),Form);});}
std::future<std::shared_ptr<const MediaDocument>> CprReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    const auto source=context.dataSource();const auto header=readSource(source,0,CprHeaderLength,token);if(!starts(header,Riff)||!starts(M::Bytes(header).subspan(FormOffset),Form))throw MediaError(DiagnosticCode::CprHeader);
    const auto end=std::uint64_t(FormOffset)+little<std::uint32_t>(header,SizeOffset);if(end>source->length()||end<CprHeaderLength)throw MediaError(DiagnosticCode::CprLength);
    struct Bank {std::int32_t number;std::string id;std::uint64_t offset;std::int32_t length;};std::map<std::int32_t,Bank> banks;std::vector<M::MessageRecipe> messages;std::uint64_t position=CprHeaderLength;
    while(position+ChunkHeaderLength<=end){
        const auto chunk=readSource(source,position,ChunkHeaderLength,token);std::string id;for(const auto byte:M::Bytes(chunk).first(SignatureLength))id.push_back(byte>AsciiMaximum?AsciiReplacement:char(byte));const auto length=little<std::uint32_t>(chunk,SizeOffset);position+=ChunkHeaderLength;const auto paddedLength=std::uint64_t(length)+(length&1);
        if(paddedLength>end-position)throw MediaError(DiagnosticCode::CprChunkRange,{id});
        std::int32_t bank=-1;const bool valid=id.size()==SignatureLength&&id.starts_with(CprBankPrefix)&&M::MetadataNumbers::decimal(std::string_view(id).substr(BankNumberOffset),bank)&&bank>=0&&bank<MaximumBankCount;
        if(valid){if(!banks.emplace(bank,Bank{bank,id,position,std::int32_t(std::min(std::uint32_t(BankLength),length))}).second)throw MediaError(DiagnosticCode::CprDuplicate,{id});if(length>BankLength)messages.push_back(message(DiagnosticCode::CprLargeBank,{id}));}
        else messages.push_back(message(DiagnosticCode::CprUnknownChunk,{id}));position+=paddedLength;
    }
    if(position!=end)throw MediaError(DiagnosticCode::CprChunkHeader);if(banks.empty())throw MediaError(DiagnosticCode::CprNoBanks);
    const auto capacity=(std::int64_t(banks.rbegin()->first)+1)*BankLength;std::vector<DataRange> ranges;std::map<std::string,std::string> metadata={{std::string(SystemIdKey),std::string(AmstradSystem)},{std::string(RoleKey),std::string(CartridgeRole)},{std::string(BankSizeKey),std::to_string(BankLength)},{std::string(BankCountKey),std::to_string(banks.size())}};
    for(const auto& [number,bank]:banks){if(bank.length>0)ranges.emplace_back(std::int64_t(number)*BankLength,bank.length,DataRangeKind::Stored,source,std::int64_t(bank.offset));metadata[bankKey(number,ChunkIdSuffix)]=bank.id;metadata[bankKey(number,BankLengthSuffix)]=std::to_string(bank.length);}
    return std::make_shared<MediaDocument>(context.source(),std::string(Formats::AmstradCpr),MediaKind::Cartridge,std::make_shared<BlockImage>(capacity,1,std::move(ranges)),std::vector<M::VolumeDescriptor>{volume(capacity,CprVolumeKey)},std::move(messages),std::move(metadata));
});}
}
