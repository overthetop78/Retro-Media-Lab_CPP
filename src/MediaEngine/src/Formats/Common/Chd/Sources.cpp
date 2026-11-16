#include "Sources.h"
#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Chd {
namespace {
using Data=MediaFileSystems::RandomAccessData;
class Decoded final:public Data {
public:
    Decoded(std::shared_ptr<const Data> source,std::stop_token token):container_(std::move(source),token),hunk_(container_.hunkBytes()){}
    std::uint64_t length()const noexcept override{return container_.logicalBytes();}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{
        checkCancellation(token);if(offset>length()||output.size()>length()-offset)throw MediaError(DiagnosticCode::ChdRange);std::scoped_lock lock(mutex_);
        while(!output.empty()){checkCancellation(token);const auto index=std::uint32_t(offset/hunk_.size());const auto position=std::size_t(offset%hunk_.size());if(!cached_||*cached_!=index){cached_.reset();container_.readHunk(index,hunk_,token);cached_=index;}
            const auto count=std::min(output.size(),hunk_.size()-position);std::copy_n(hunk_.begin()+position,count,output.begin());offset+=count;output=output.subspan(count);}
    }
private:mutable Container container_;mutable std::vector<std::uint8_t> hunk_;mutable std::optional<std::uint32_t> cached_;mutable std::mutex mutex_;
};
class Uncompressed final:public Data {
public:
    Uncompressed(std::shared_ptr<const Data> source,std::uint64_t logical,std::uint32_t hunk,std::vector<std::uint32_t> map):source_(std::move(source)),length_(logical),hunkBytes_(hunk),map_(std::move(map)){
        if(!source_||!length_||!hunkBytes_||map_.size()!=length_/hunkBytes_+(length_%hunkBytes_!=0))throw MediaError(DiagnosticCode::ChdDimensions);
        for(auto stored:map_){if(!stored)continue;const auto offset=std::uint64_t(stored)*hunkBytes_;if(offset>source_->length()||hunkBytes_>source_->length()-offset)throw MediaError(DiagnosticCode::ChdMap);}
    }
    std::uint64_t length()const noexcept override{return length_;}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{
        checkCancellation(token);if(offset>length_||output.size()>length_-offset)throw MediaError(DiagnosticCode::ChdRange);
        while(!output.empty()){checkCancellation(token);const auto index=std::size_t(offset/hunkBytes_),position=std::size_t(offset%hunkBytes_),count=std::min(output.size(),std::size_t(hunkBytes_)-position);auto target=output.first(count);const auto stored=map_[index];
            if(!stored)std::fill(target.begin(),target.end(),0);else source_->readExactly(std::uint64_t(stored)*hunkBytes_+position,target,token);offset+=count;output=output.subspan(count);}
    }
private:std::shared_ptr<const Data> source_;std::uint64_t length_;std::uint32_t hunkBytes_;std::vector<std::uint32_t> map_;
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> decodedSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){return std::make_shared<Decoded>(std::move(source),token);}
std::shared_ptr<const MediaFileSystems::RandomAccessData> uncompressedSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::uint64_t logical,std::uint32_t hunk,std::vector<std::uint32_t> map){return std::make_shared<Uncompressed>(std::move(source),logical,hunk,std::move(map));}
}
