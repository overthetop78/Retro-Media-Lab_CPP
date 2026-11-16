#include "Internal.h"
#include "ConsoleConstants.h"
namespace RetroMediaLab::MediaEngine::Cartridge {
namespace {
using namespace ConsoleConstants;
const ExtensionFormat* extensionFormat(std::string_view extension){const auto found=std::find_if(Extensions.begin(),Extensions.end(),[&](const auto& entry){return sameIdentifier(extension,entry.extension);});return found==Extensions.end()?nullptr:&*found;}
}
const ReaderCapabilities& ConsoleReader::capabilities()const{
    static const ReaderCapabilities result=[] {ReaderCapabilities value;for(const auto& entry:ConsoleConstants::Extensions){value.extensions.emplace_back(entry.extension);if(std::none_of(value.formatIds.begin(),value.formatIds.end(),[&](const auto& id){return sameIdentifier(id,entry.format);}))value.formatIds.emplace_back(entry.format);}value.mediaKinds={MediaKind::Cartridge};value.representationKinds={RepresentationKind::Blocks};return value;}();return result;
}
std::future<bool> ConsoleReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);const auto* format=extensionFormat(context.extension());return context.length()>0&&format&&(!context.requestedFormatId()||sameIdentifier(*context.requestedFormatId(),format->format));});}
std::future<std::shared_ptr<const MediaDocument>> ConsoleReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]()->std::shared_ptr<const MediaDocument>{
    checkCancellation(token);const auto* selected=extensionFormat(context.extension());if(!selected||context.length()<=0)throw MediaError(DiagnosticCode::ConsoleCartridgeRead);
    if(context.requestedFormatId()&&!sameIdentifier(*context.requestedFormatId(),selected->format))throw MediaError(DiagnosticCode::ConsoleCartridgeMismatch);
    const auto count=(context.length()-1)/Constants::BankLength+1;if(count>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::ConsoleCartridgeLimit);
    const auto profile=std::find_if(ConsoleConstants::Profiles.begin(),ConsoleConstants::Profiles.end(),[&](const auto& value){return value.format==selected->format;});
    const auto capacity=count*Constants::BankLength;std::vector<DataRange> ranges;ranges.reserve(static_cast<std::size_t>(count));
    std::map<std::string,std::string> metadata{{std::string(Constants::SystemIdKey),std::string(profile->system)},{std::string(Constants::RoleKey),std::string(Constants::CartridgeRole)},{std::string(ConsoleConstants::FormatKey),std::string(selected->format)},{std::string(Constants::BankSizeKey),std::to_string(Constants::BankLength)},{std::string(Constants::BankCountKey),std::to_string(count)},{std::string(ConsoleConstants::StoredLengthKey),std::to_string(context.length())},{std::string(ConsoleConstants::ExtensionKey),context.extension()}};
    const auto source=context.dataSource();
    for(std::int32_t bank=0;bank<count;++bank){
        checkCancellation(token);const auto offset=std::int64_t(bank)*Constants::BankLength;const auto length=std::min<std::int64_t>(Constants::BankLength,context.length()-offset);ranges.emplace_back(offset,length,DataRangeKind::Stored,source,offset);
        metadata[Internal::bankKey(bank,Constants::BankLengthSuffix)]=std::to_string(length);metadata[Internal::bankKey(bank,ConsoleConstants::BankNameSuffix)]=std::string(ConsoleConstants::BankNamePrefix)+Internal::padded(bank);
    }
    MediaFileSystems::VolumeDescriptor volume(0,capacity,std::string(MediaFileSystems::VolumeConstants::DirectVolume),{},{},{},{},{},{},{},std::string(profile->name));
    return std::make_shared<MediaDocument>(context.source(),std::string(selected->format),MediaKind::Cartridge,std::make_shared<BlockImage>(capacity,Constants::BankLength,std::move(ranges)),std::vector<MediaFileSystems::VolumeDescriptor>{std::move(volume)},std::vector<MediaFileSystems::MessageRecipe>{},std::move(metadata));
});}
}
