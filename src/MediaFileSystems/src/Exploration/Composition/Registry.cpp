#include "Internal.h"
#include <map>
#include <set>
namespace RetroMediaLab::MediaFileSystems {
struct SectorRegistry::Impl {
    std::vector<std::shared_ptr<const SectorReader>> readers;
    std::vector<std::string> formats;
    std::map<std::u16string,std::shared_ptr<const SectorReader>> byId;
    std::map<std::u16string,std::vector<std::shared_ptr<const SectorReader>>> byFormat;
};
namespace {
FileSystemReadReport run(const SectorImage& image,std::span<const std::shared_ptr<const SectorReader>> readers,std::stop_token token){
    FileSystemReadReport report;
    for(const auto& reader:readers){checkCancellation(token);if(!reader->canRead(image,token))continue;
        try{report.matches.push_back({std::string(reader->id()),reader->read(image,token)});}
        catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData)throw;report.failures.push_back({std::string(reader->id()),std::current_exception()});}}
    return report;
}
}
SectorRegistry::SectorRegistry():SectorRegistry(createDefaultSectorReaders()){}
SectorRegistry::SectorRegistry(std::vector<std::shared_ptr<const SectorReader>> readers){
    auto impl=std::make_shared<Impl>();impl->readers=std::move(readers);
    for(std::size_t index=0;index<impl->readers.size();++index){const auto& reader=impl->readers[index];if(!reader)throw MediaError(DiagnosticCode::RegistryNullReader,{std::to_string(index)});if(Composition::blank(reader->id()))throw MediaError(DiagnosticCode::RegistryEmptyReaderId,{std::to_string(index)});}
    std::map<std::u16string,std::size_t> counts;for(const auto& reader:impl->readers)++counts[Composition::ordinalKey(reader->id())];
    for(const auto& reader:impl->readers){const auto key=Composition::ordinalKey(reader->id());if(counts.at(key)>1)throw MediaError(DiagnosticCode::RegistryDuplicateReaderId,{std::string(reader->id())});impl->byId.emplace(key,reader);}
    for(const auto& reader:impl->readers)for(const auto format:reader->catalogFormatIds()){auto key=Composition::ordinalKey(format);auto [it,added]=impl->byFormat.try_emplace(std::move(key));if(added)impl->formats.emplace_back(format);it->second.push_back(reader);}
    impl_=std::move(impl);
}
std::span<const std::shared_ptr<const SectorReader>> SectorRegistry::readers() const noexcept{return impl_->readers;}
std::span<const std::string> SectorRegistry::supportedFormatIds() const noexcept{return impl_->formats;}
FileSystemReadReport SectorRegistry::readAll(const SectorImage& image,std::stop_token token) const{return run(image,impl_->readers,token);}
FileSystemReadReport SectorRegistry::readCandidates(const SectorImage& image,std::optional<std::string_view> selected,std::stop_token token) const{
    if(!selected)return readAll(image,token);const auto key=Composition::ordinalKey(*selected);
    if(const auto it=impl_->byId.find(key);it!=impl_->byId.end()){const std::shared_ptr<const SectorReader> reader=it->second;return run(image,std::span(&reader,1),token);}
    if(const auto it=impl_->byFormat.find(key);it!=impl_->byFormat.end())return run(image,it->second,token);return {};
}
std::optional<FileSystemMatch> SectorRegistry::tryRead(const SectorImage& image,std::optional<std::string_view> selected,std::stop_token token) const{auto report=readCandidates(image,selected,token);if(report.matches.empty())return {};return std::move(report.matches.front());}
std::vector<SectorFileSystemCandidate> SectorRegistry::readDistinctCandidates(std::span<const std::shared_ptr<const SectorImage>> images,std::stop_token token) const{
    std::vector<SectorFileSystemCandidate> result;std::set<std::u16string> identities;bool primary=true;
    for(const auto& image:images){checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::RegistryNullImage);const auto it=impl_->byFormat.find(Composition::ordinalKey(image->geometry().formatId));auto report=it==impl_->byFormat.end()?readAll(*image,token):run(*image,it->second,token);
        const auto count=primary?report.matches.size():std::min<std::size_t>(1,report.matches.size());for(std::size_t index=0;index<count;++index){auto& match=report.matches[index];if(identities.insert(Composition::identity(image->geometry().formatId,match.volume)).second)result.push_back({image,std::move(match)});}primary=false;}
    return result;
}
}
