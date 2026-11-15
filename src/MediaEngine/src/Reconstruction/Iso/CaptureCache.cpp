#include "CandidateDecoder.h"
#include "Constants.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <future>
#include <mutex>
#include <unordered_map>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace {
struct Fingerprint {
    std::filesystem::path path;
    bool exists=false;
    std::uintmax_t length=0;
    std::filesystem::file_time_type modified{};
    bool operator==(const Fingerprint&) const = default;
};
std::string utf8(const std::filesystem::path& path){
    const auto value=path.u8string();
    return {value.begin(),value.end()};
}
Fingerprint fingerprint(const std::filesystem::path& path){
    if(path.empty()||path.native().find(std::filesystem::path::value_type{})!=std::filesystem::path::string_type::npos)
        throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(path)});
    std::error_code error;
    auto fullPath=std::filesystem::absolute(path,error).lexically_normal();
    if(error)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(path)});
    Fingerprint result{std::move(fullPath)};
    if(!std::filesystem::is_regular_file(result.path,error)||error)return result;
    const auto length=std::filesystem::file_size(result.path,error);
    if(error)return result;
    const auto modified=std::filesystem::last_write_time(result.path,error);
    if(error)return result;
    result.exists=true;result.length=length;result.modified=modified;
    return result;
}
std::string decoderKey(std::span<const std::string> ids){
    std::string key;
    for(std::size_t index=0;index<ids.size();++index){
        if(index)key+=Constants::DecoderKeySeparator;
        key+=ids[index];
    }
    return key;
}
struct CacheEntry {
    explicit CacheEntry(Fingerprint value):capture(std::move(value)),result(promise.get_future().share()){}
    Fingerprint capture;
    std::promise<SharedCandidates> promise;
    std::shared_future<SharedCandidates> result;
};
}
class CandidateDecoder::State {
public:
    State(DecodeFlux decode,ReadCapture read):decodeFlux(std::move(decode)),readCapture(std::move(read)){
        if(!readCapture)readCapture=[](const auto& path,auto token){return Scp::read(*fileSource(path),token);};
    }
    DecodeFlux decodeFlux;
    ReadCapture readCapture;
    std::mutex gate;
    std::unordered_map<std::string,std::shared_ptr<CacheEntry>> entries;
};
CandidateDecoder::CandidateDecoder(DecodeFlux decode,ReadCapture read):state_(std::make_unique<State>(std::move(decode),std::move(read))){}
CandidateDecoder::~CandidateDecoder()=default;
SharedCandidates CandidateDecoder::decode(const Flux::ProtectedTrackImage& image,std::span<const std::string> ids,const CollectionProgressCallback& progress,std::stop_token token) const {
    return std::make_shared<const CandidateSet>(collect(image,ids,state_->decodeFlux,progress,token));
}
SharedCandidates CandidateDecoder::decode(const std::filesystem::path& path,std::span<const std::string> ids,const CollectionProgressCallback& progress,std::stop_token token){
    const auto capture=fingerprint(path);
    const auto key=decoderKey(ids);
    std::shared_ptr<CacheEntry> entry;
    bool ownsOperation=false;
    {
        std::lock_guard lock(state_->gate);
        const auto found=state_->entries.find(key);
        if(found==state_->entries.end()||found->second->capture!=capture){
            entry=std::make_shared<CacheEntry>(capture);
            state_->entries[key]=entry;
            ownsOperation=true;
        }else entry=found->second;
    }
    if(ownsOperation){
        try{
            const auto scp=state_->readCapture(capture.path,token);
            const auto image=Scp::protectedImage(scp,token);
            entry->promise.set_value(decode(image,ids,progress,token));
        }catch(...){
            entry->promise.set_exception(std::current_exception());
        }
    }
    try{
        return entry->result.get();
    }catch(...){
        // An older operation must never evict a replacement for the same codec list.
        std::lock_guard lock(state_->gate);
        const auto found=state_->entries.find(key);
        if(found!=state_->entries.end()&&found->second==entry)state_->entries.erase(found);
        throw;
    }
}
}
