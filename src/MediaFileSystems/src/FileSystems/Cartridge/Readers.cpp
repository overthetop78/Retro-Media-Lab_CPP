#include "Constants.h"
#include <RetroMediaLab/MediaFileSystems/CartridgeReaders.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <QString>
#include "../../Primitives/Metadata.h"
#include <limits>

namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace CartridgeConstants;
using Metadata::metadataInt;
std::string bankKey(int bank,std::string_view suffix) {
    return std::string(BankPrefix)+std::to_string(bank)+std::string(suffix);
}
template<std::size_t N> bool supported(std::string_view format,const std::array<std::string_view,N>& formats) {
    const auto value=QString::fromUtf8(format.data(),qsizetype(format.size()));
    for(const auto candidate:formats)
        if(value.compare(QString::fromUtf8(candidate.data(),qsizetype(candidate.size())),Qt::CaseInsensitive)==0)return true;
    return false;
}
bool whole(const BlockRepresentation* blocks,const VolumeDescriptor& volume) {
    return blocks&&blocks->capacity()>FirstBank&&volume.start()==FirstBank&&volume.length()==std::uint64_t(blocks->capacity());
}
FileSystemVolume makeVolume(const ImageDocument& document,const VolumeDescriptor& descriptor,const BlockRepresentation& blocks,std::string_view id) {
    FileSystemVolume volume;
    volume.name=descriptor.name().value_or(std::string{});
    volume.fileSystemId=id;
    volume.capacity=std::uint64_t(blocks.capacity());
    volume.freeSpaceKnown=false;
    volume.attributes={document.formatId(),std::string(BanksAttribute)};
    const auto diagnostics=document.diagnostics();
    volume.sourceDiagnostics.assign(diagnostics.begin(),diagnostics.end());
    const auto messages=document.messages();volume.sourceMessages.assign(messages.begin(),messages.end());
    volume.nameRecipe=descriptor.nameRecipe();
    return volume;
}
FileSystemEntry readBank(const ImageDocument& document,const BlockRepresentation& blocks,int bank,int bankSize,int length,std::string_view nameSuffix,bool checkedReference,std::stop_token token) {
    checkCancellation(token);
    const auto address=std::int64_t(bank)*bankSize;
    std::vector<std::uint8_t> data(std::size_t(length),std::uint8_t{});
    blocks.readExactlyAsync(address,data,token).get();
    // C# performs checked(bank * bankSize) after the read has completed.
    if(checkedReference&&address>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::ConsoleCartridgeBankAddress);
    const auto stored=document.metadata().find(bankKey(bank,nameSuffix));
    auto number=std::to_string(bank);
    if(number.size()<std::size_t(NameDigits))number.insert(number.begin(),std::size_t(NameDigits)-number.size(),Zero);
    FileSystemEntry entry;
    entry.name=stored==document.metadata().end()?std::string(DefaultNamePrefix)+number:stored->second;
    entry.kind=EntryKind::File;
    entry.size=std::uint64_t(length);
    entry.storageReference=address;
    entry.content=std::move(data);
    entry.nativeTypeId=BankEntryType;
    entry.occupiedSize=entry.size;
    entry.attributes={std::string(BankAttribute)};
    entry.dataValid=true;
    entry.syntheticName=stored==document.metadata().end();
    entry.metadata={{std::string(BankNumberMetadata),std::to_string(bank)},{std::string(AddressMetadata),std::to_string(address)}};
    return entry;
}
bool validConsoleMetadata(const ImageDocument& document) {
    int count{},size{};
    if(!metadataInt(document.metadata(),std::string(BankCount),count)||count<=FirstBank||count>MaximumBankCount||!metadataInt(document.metadata(),std::string(BankSize),size)||size<=FirstBank)return false;
    for(int bank=FirstBank;bank<count;++bank){int length{};if(!metadataInt(document.metadata(),bankKey(bank,LengthSuffix),length)||length<=FirstBank||length>size)return false;}
    return true;
}
}
std::string_view AmstradCartridgeReader::id() const noexcept {return FileSystemIds::AmstradMediaStructure;}
bool AmstradCartridgeReader::canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    checkCancellation(token);
    return supported(document.formatId(),CartridgeConstants::AmstradFormats)&&whole(dynamic_cast<const BlockRepresentation*>(&document.representation()),volume);
}
FileSystemVolume AmstradCartridgeReader::read(const ImageDocument& document,const VolumeDescriptor& descriptor,std::stop_token token) const {
    using namespace CartridgeConstants;
    if(!canRead(document,descriptor,token))throw MediaError(DiagnosticCode::AmstradCartridgeUnsupported);
    const auto& blocks=dynamic_cast<const BlockRepresentation&>(document.representation());
    int count{};
    if(!metadataInt(document.metadata(),std::string(BankCount),count)||count<=FirstBank||count>AmstradBankCount)throw MediaError(DiagnosticCode::AmstradCartridgeBankCount);
    auto volume=makeVolume(document,descriptor,blocks,id());
    volume.fileSystemDisplayResourceKey=AmstradDisplayKey;
    volume.fileSystemDisplayTranslationDomain=TranslationDomain;
    volume.entries.reserve(std::size_t(count));
    for(int bank=FirstBank;bank<AmstradBankCount;++bank){
        int length{};
        if(!metadataInt(document.metadata(),bankKey(bank,LengthSuffix),length))continue;
        if(length<=FirstBank||length>AmstradBankLength)throw MediaError(DiagnosticCode::AmstradCartridgeBankLength,{std::to_string(bank)});
        volume.entries.push_back(readBank(document,blocks,bank,AmstradBankLength,length,ChunkIdSuffix,false,token));
    }
    if(volume.entries.size()!=std::size_t(count))throw MediaError(DiagnosticCode::AmstradCartridgeBankMismatch);
    return volume;
}
std::string_view ConsoleCartridgeReader::id() const noexcept {return FileSystemIds::ConsoleCartridgeStructure;}
bool ConsoleCartridgeReader::canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token) const {
    checkCancellation(token);
    return supported(document.formatId(),CartridgeConstants::ConsoleFormats)&&whole(dynamic_cast<const BlockRepresentation*>(&document.representation()),volume)&&validConsoleMetadata(document);
}
FileSystemVolume ConsoleCartridgeReader::read(const ImageDocument& document,const VolumeDescriptor& descriptor,std::stop_token token) const {
    using namespace CartridgeConstants;
    if(!canRead(document,descriptor,token))throw MediaError(DiagnosticCode::ConsoleCartridgeUnsupported);
    const auto& blocks=dynamic_cast<const BlockRepresentation&>(document.representation());
    int count{},size{};
    if(!metadataInt(document.metadata(),std::string(BankCount),count)||count<=FirstBank||count>MaximumBankCount)throw MediaError(DiagnosticCode::ConsoleCartridgeBankCount);
    if(!metadataInt(document.metadata(),std::string(BankSize),size)||size<=FirstBank)throw MediaError(DiagnosticCode::ConsoleCartridgeBankSize);
    auto volume=makeVolume(document,descriptor,blocks,id());
    volume.fileSystemDisplayName=descriptor.name().value_or(document.formatId());
    if(descriptor.nameRecipe()){const auto& recipe=*descriptor.nameRecipe();volume.fileSystemDisplayResourceKey=recipe.resourceKey;volume.fileSystemDisplayTranslationDomain=recipe.translationDomain;volume.fileSystemDisplayArguments=recipe.arguments;}
    volume.entries.reserve(std::size_t(count));
    for(int bank=FirstBank;bank<count;++bank){
        int length{};
        if(!metadataInt(document.metadata(),bankKey(bank,LengthSuffix),length)||length<=FirstBank||length>size)throw MediaError(DiagnosticCode::ConsoleCartridgeBankLength,{std::to_string(bank)});
        volume.entries.push_back(readBank(document,blocks,bank,size,length,NameSuffix,true,token));
    }
    return volume;
}
}
