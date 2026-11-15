#include "Internal.h"
#include "../../Sources/RandomAccessSources.h"
#include <set>
namespace RetroMediaLab::MediaEngine::Cartridge {
using namespace Constants;using namespace Internal;
namespace {
struct Bank {std::int32_t number,length;DataRange range;};
std::optional<std::vector<Bank>> cprBanks(const MediaDocument& document){
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());if(!blocks||blocks->capacity()<=0||blocks->capacity()>std::int64_t(MaximumBankCount)*BankLength)return {};
    std::vector<Bank> result;std::set<std::int32_t> seen;
    for(const auto& range:blocks->ranges()){
        if(range.kind()!=DataRangeKind::Stored||!range.source()||range.address()%BankLength||range.length()<=0||range.length()>BankLength)return {};
        const auto number=integer(range.address()/BankLength);std::int32_t stored{};const auto length=M::MetadataNumbers::metadataInt(document.metadata(),bankKey(number,BankLengthSuffix),stored)?stored:integer(range.length());if(length<=0||length>range.length()||!seen.insert(number).second)return {};
        result.push_back({number,length,range});
    }
    if(result.empty())return {};return result;
}
std::optional<std::vector<DataRange>> romBanks(const MediaDocument& document){
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());if(!blocks||blocks->ranges().empty())return {};
    std::vector<DataRange> ranges(blocks->ranges().begin(),blocks->ranges().end());
    for(std::size_t index=0;index<ranges.size();++index){const auto& range=ranges[index];if(range.kind()!=DataRangeKind::Stored||!range.source()||range.address()!=std::int64_t(index)*BankLength||range.length()<=0||range.length()>BankLength||(index+1<ranges.size()&&range.length()!=BankLength))return {};}
    return ranges;
}
struct Sides {std::vector<DataRange> ranges;bool headered;};
std::optional<Sides> fdsSides(const MediaDocument& document){
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());if(!blocks||blocks->ranges().empty()||blocks->logicalBlockSize()!=FdsSideLength||blocks->capacity()%FdsSideLength)return {};
    bool headered=false;std::int32_t stored{};if(M::MetadataNumbers::metadataInt(document.metadata(),std::string(HeaderLengthKey),stored)){if(stored!=0&&stored!=FdsHeaderLength)return {};headered=stored==FdsHeaderLength;}
    const auto count=integer(blocks->capacity()/FdsSideLength);if(blocks->ranges().size()!=std::size_t(count))return {};std::vector<DataRange> ranges(blocks->ranges().begin(),blocks->ranges().end());
    for(std::size_t side=0;side<ranges.size();++side){const auto& range=ranges[side];if(range.kind()!=DataRangeKind::Stored||!range.source()||range.address()!=std::int64_t(side)*FdsSideLength||range.length()!=FdsSideLength)return {};}
    return Sides{std::move(ranges),headered};
}
bool canWrite(Kind kind,const MediaDocument& document,std::string_view format,std::string_view extension){
    switch(kind){
    case Kind::Cpr:return sameIdentifier(format,Formats::AmstradCpr)&&sameIdentifier(extension,CprExtension)&&cprBanks(document).has_value();
    case Kind::Rom:return sameIdentifier(format,Formats::AmstradRom)&&(sameIdentifier(extension,RomExtension)||sameIdentifier(extension,BinExtension))&&romBanks(document).has_value();
    case Kind::Fds:return sameIdentifier(format,Formats::NintendoFamicomDisk)&&sameIdentifier(extension,FdsExtension)&&fdsSides(document).has_value();
    }
    return false;
}
std::string extension(const std::filesystem::path& path){const auto text=path.extension().u8string();return std::string(text.begin(),text.end());}
void emitCpr(const ImageOutput& output,const std::vector<Bank>& banks,std::stop_token token){
    std::uint32_t size=SignatureLength;for(const auto& bank:banks)size+=ChunkHeaderLength+std::uint32_t(bank.length)+(bank.length&1);
    std::vector<std::uint8_t> header(CprHeaderLength);std::copy(Riff.begin(),Riff.end(),header.begin());little<std::uint32_t>(header,SizeOffset,size);std::copy(Form.begin(),Form.end(),header.begin()+FormOffset);output(header);
    for(const auto& bank:banks){M::checkCancellation(token);std::vector<std::uint8_t> chunk(ChunkHeaderLength);const auto id=std::string(CprBankPrefix)+padded(bank.number);std::copy(id.begin(),id.end(),chunk.begin());little<std::uint32_t>(chunk,SizeOffset,std::uint32_t(bank.length));output(chunk);copyStoredRange(DataRange(bank.range.address(),bank.length,DataRangeKind::Stored,bank.range.source(),bank.range.sourceOffset()),output,token);if(bank.length&1){const std::array<std::uint8_t,1> padding{};output(padding);}}
}
std::vector<std::filesystem::path> write(Kind kind,const std::shared_ptr<const MediaDocument>& document,const std::filesystem::path& path,std::string_view format,std::stop_token token){
    const auto failure=kind==Kind::Cpr?DiagnosticCode::CprWrite:kind==Kind::Rom?DiagnosticCode::RomWrite:DiagnosticCode::FdsWrite;
    if(!document||!canWrite(kind,*document,format,extension(path)))throw MediaError(failure);
    if(kind==Kind::Cpr){const auto banks=cprBanks(*document);if(!banks)throw MediaError(failure);writeAtomicImage(path,[&](const auto& output,std::stop_token current){emitCpr(output,*banks,current);},token);}
    else if(kind==Kind::Rom){const auto banks=romBanks(*document);if(!banks)throw MediaError(failure);writeAtomicImage(path,[&](const auto& output,std::stop_token current){for(const auto& bank:*banks)copyStoredRange(bank,output,current);},token);}
    else {const auto sides=fdsSides(*document);if(!sides)throw MediaError(failure);writeAtomicImage(path,[&](const auto& output,std::stop_token current){if(sides->headered){const auto raw=fileSource(sourcePath(document->source().primaryPath()));const auto header=readSource(raw,0,FdsHeaderLength,current);if(!fdsSignature(header))throw MediaError(DiagnosticCode::FdsHeader);output(header);}for(const auto& side:sides->ranges)copyStoredRange(side,output,current);},token);}
    return {path};
}
}
const WriterCapabilities& Writer::capabilities()const{
    static const WriterCapabilities cpr{std::string(CprWriterId),{std::string(Formats::AmstradCpr)},{std::string(CprExtension)},{RepresentationKind::Blocks},false};
    static const WriterCapabilities rom{std::string(RomWriterId),{std::string(Formats::AmstradRom)},{std::string(RomExtension),std::string(BinExtension)},{RepresentationKind::Blocks},false};
    static const WriterCapabilities fds{std::string(FdsWriterId),{std::string(Formats::NintendoFamicomDisk)},{std::string(FdsExtension)},{RepresentationKind::Blocks},false};
    return kind_==Kind::Cpr?cpr:kind_==Kind::Rom?rom:fds;
}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{return Cartridge::canWrite(kind_,document,format,extension);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return std::async(std::launch::deferred,[kind=kind_,document=std::move(document),output=std::move(output),target=std::move(target),token]{return write(kind,document,output,target,token);});}
}

