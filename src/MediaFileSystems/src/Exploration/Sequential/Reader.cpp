#include <RetroMediaLab/MediaFileSystems/SequentialContent.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include "Constants.h"
#include <QString>
#include <charconv>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems {
namespace {
using namespace SequentialConstants;
std::optional<int> blockNumber(const SequentialDecodedBlock& block) {
    const auto found=block.metadata().find(std::string(BlockNumberKey));if(found==block.metadata().end())return {};
    const auto& text=found->second;if(text.empty()||!std::all_of(text.begin(),text.end(),[](char value){return value>=FirstDigit&&value<=LastDigit;}))return {};
    int number=0;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),number);
    if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size())return {};return number;
}
}
std::string_view SequentialContentReader::id()const noexcept{return FileSystemIds::SequentialContent;}
FileSystemVolume SequentialContentReader::read(const ImageDocument& document,const VolumeDescriptor& descriptor,const SequentialContent& content,std::string volumeName,std::stop_token token)const {
    checkCancellation(token);FileSystemVolume volume;volume.name=std::move(volumeName);volume.fileSystemId=id();volume.capacity=descriptor.length();volume.freeSpaceKnown=false;
    volume.attributes.push_back(document.formatId());if(const auto decoder=content.decoderId())volume.attributes.push_back(*decoder);
    for(const auto& diagnostic:content.diagnostics())if(std::find(volume.sourceDiagnostics.begin(),volume.sourceDiagnostics.end(),diagnostic)==volume.sourceDiagnostics.end())volume.sourceDiagnostics.push_back(diagnostic);
    for(const auto& message:content.messages())if(std::find(volume.sourceMessages.begin(),volume.sourceMessages.end(),message)==volume.sourceMessages.end())volume.sourceMessages.push_back(message);
    std::vector<const SequentialDecodedBlock*> current;std::optional<std::string> currentName;std::optional<int> previousNumber;std::size_t anonymous=0;bool invalidBlock=false;
    const auto flush=[&] {
        if(current.empty()||!currentName)return;
        FileSystemEntry entry;entry.name=*currentName;entry.kind=EntryKind::File;entry.content.emplace();bool allValid=true,anyInvalid=false;
        for(const auto* block:current) {
            checkCancellation(token);const auto data=block->data();if(data.size()>std::size_t(MaximumFileBytes)-entry.content->size())throw MediaError(DiagnosticCode::VolumeRangeOverflow);
            entry.content->insert(entry.content->end(),data.begin(),data.end());const auto valid=block->integrityValid();allValid=allValid&&valid.value_or(false);anyInvalid=anyInvalid||(valid&&!*valid);
        }
        entry.size=entry.content->size();const auto& first=*current.front();entry.storageReference=std::min(first.position(),MaximumStorageReference);entry.metadata=first.metadata();
        if(const auto native=first.metadata().find(std::string(FileTypeKey));native!=first.metadata().end())entry.nativeTypeId=native->second;
        if(anyInvalid)entry.dataValid=false;else if(allValid)entry.dataValid=true;
        if(anyInvalid)entry.diagnostics.push_back({DiagnosticCode::SequentialFileInvalid,{}});
        volume.entries.push_back(std::move(entry));current.clear();currentName.reset();previousNumber.reset();
    };
    for(const auto& block:content.blocks()) {
        checkCancellation(token);if(const auto valid=block.integrityValid();valid&&!*valid)invalidBlock=true;
        const auto name=block.metadata().find(std::string(FileNameKey));const auto stored=name==block.metadata().end()?QString{}:QString::fromStdString(name->second).trimmed();
        if(stored.isEmpty()){flush();++anonymous;continue;}
        const auto value=stored.toStdString();const auto number=blockNumber(block);
        if(!current.empty()&&(*currentName!=value||!number||!previousNumber||std::int64_t(*number)!=std::int64_t(*previousNumber)+1))flush();
        currentName=value;current.push_back(&block);previousNumber=number;
    }
    flush();if(anonymous)volume.warnings.push_back({DiagnosticCode::SequentialAnonymous,{std::to_string(anonymous)}});
    if(invalidBlock)volume.warnings.push_back({DiagnosticCode::SequentialIntegrity,{}});
    if(volume.entries.empty())volume.warnings.push_back({DiagnosticCode::SequentialNoFiles,{}});
    return volume;
}
}
