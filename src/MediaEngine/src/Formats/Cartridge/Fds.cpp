#include "Internal.h"
namespace RetroMediaLab::MediaEngine::Cartridge {
using namespace Constants;using namespace Internal;
const ReaderCapabilities& FdsReader::capabilities()const{static const ReaderCapabilities value{{std::string(Formats::NintendoFamicomDisk)},{std::string(FdsExtension)},{std::vector<std::uint8_t>(FdsSignature.begin(),FdsSignature.end())},{},{MediaKind::Floppy},{RepresentationKind::Blocks}};return value;}
std::future<bool> FdsReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    M::checkCancellation(token);if(!sameIdentifier(context.extension(),FdsExtension)||!requested(context,Formats::NintendoFamicomDisk))return false;
    if(context.length()>=FdsHeaderLength){const auto header=context.readHeaderAsync(FdsHeaderLength,token).get();if(fdsSignature(header)){const auto length=context.length()-FdsHeaderLength;if(length<=0||length%FdsSideLength)return false;const auto count=length/FdsSideLength,declared=std::int64_t(header[FdsSideCountOffset]);return count>=1&&(!declared||count==declared);}}
    return context.length()>=FdsSideLength&&context.length()%FdsSideLength==0;
});}
std::future<std::shared_ptr<const MediaDocument>> FdsReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    M::checkCancellation(token);if(!sameIdentifier(context.extension(),FdsExtension)||!requested(context,Formats::NintendoFamicomDisk))throw MediaError(DiagnosticCode::FdsSource);
    const auto header=context.length()>=FdsHeaderLength?context.readHeaderAsync(FdsHeaderLength,token).get():std::vector<std::uint8_t>{};const bool headered=header.size()==FdsHeaderLength&&fdsSignature(header);const auto headerLength=headered?FdsHeaderLength:0;const auto length=context.length()-headerLength;
    if(length<=0||length%FdsSideLength)throw MediaError(DiagnosticCode::FdsSides);const auto count=integer(length/FdsSideLength);if(count<1)throw MediaError(DiagnosticCode::FdsNoSide);const auto declared=headered?header[FdsSideCountOffset]:0;if(headered&&declared&&declared!=count)throw MediaError(DiagnosticCode::FdsCount);
    const auto source=context.dataSource();std::vector<DataRange> ranges;std::map<std::string,std::string> metadata={{std::string(SystemIdKey),std::string(NintendoSystem)},{std::string(RoleKey),std::string(FloppyRole)},{std::string(HeaderedKey),std::string(headered?True:False)},{std::string(HeaderLengthKey),std::to_string(headerLength)},{std::string(SideCountKey),std::to_string(count)},{std::string(DeclaredSideCountKey),std::to_string(declared)},{std::string(SideLengthKey),std::to_string(FdsSideLength)}};
    for(std::int32_t side=0;side<count;++side){const auto address=std::int64_t(side)*FdsSideLength;ranges.emplace_back(address,FdsSideLength,DataRangeKind::Stored,source,headerLength+address);metadata[std::string(SidePrefix)+std::to_string(side)+std::string(SideNameSuffix)]=std::string(SideNamePrefix)+padded(side);}
    const auto capacity=std::int64_t(count)*FdsSideLength;std::vector<M::MessageRecipe> messages;if(headered&&!declared)messages.push_back(message(DiagnosticCode::FdsUndeclared));
    return std::make_shared<MediaDocument>(context.source(),std::string(Formats::NintendoFamicomDisk),MediaKind::Floppy,std::make_shared<BlockImage>(capacity,FdsSideLength,std::move(ranges)),std::vector<M::VolumeDescriptor>{volume(capacity,FdsVolumeKey)},std::move(messages),std::move(metadata));
});}
}
