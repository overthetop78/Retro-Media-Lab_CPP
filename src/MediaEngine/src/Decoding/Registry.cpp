#include "Registry.h"
#include "../Composition/FluxCodecs.h"
#include "Scoring.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Primitives/Identifiers.h"
#include <future>
#include <map>
#include <mutex>
namespace RetroMediaLab::MediaEngine::Decoding {
namespace {
struct RevolutionKey {
    std::weak_ptr<const Flux::Revolution> owner;
    const Flux::Revolution* address;
};
struct RevolutionKeyLess {
    bool operator()(const RevolutionKey& left,const RevolutionKey& right) const noexcept {
        const std::owner_less<std::weak_ptr<const Flux::Revolution>> less;
        if(less(left.owner,right.owner))return true;
        if(less(right.owner,left.owner))return false;
        return std::less<const Flux::Revolution*>{}(left.address,right.address);
    }
};
struct DeferredResult {
    DeferredResult():result(promise.get_future().share()){}
    std::promise<SharedResult> promise;
    std::shared_future<SharedResult> result;
};
}
class Registry::State {
public:
    explicit State(std::vector<std::shared_ptr<const Decoder>> decoders):ordered(std::move(decoders)){
        if(ordered.empty())throw MediaError(DiagnosticCode::RegistryEmpty);
        for(std::size_t index=0;index<ordered.size();++index){
            const auto& decoder=ordered[index];
            if(!decoder)throw MediaError(DiagnosticCode::RegistryNullDecoder,{std::to_string(index)});
            const auto id=decoder->id();
            if(Primitives::isBlankIdentifier(id))throw MediaError(DiagnosticCode::RegistryInvalidId,{std::to_string(index)});
        }
        for(const auto& decoder:ordered){
            const std::string id(decoder->id());
            if(!byId.emplace(id,decoder).second)throw MediaError(DiagnosticCode::RegistryDuplicate,{id});
            if(decoder->participatesInAutomaticDetection())automatic.push_back(decoder);
        }
    }
    SharedResult decode(std::string_view id,const Flux::SharedRevolution& revolution){
        const auto found=byId.find(std::string(id));
        if(found==byId.end())throw MediaError(DiagnosticCode::RegistryUnknown,{std::string(id)});
        if(!revolution)throw MediaError(DiagnosticCode::RegistryNullRevolution);
        const auto owned=revolution;
        std::shared_ptr<DeferredResult> deferred;
        bool execute=false;
        {
            std::lock_guard lock(gate);
            for(auto entry=cache.begin();entry!=cache.end();){
                if(entry->first.owner.expired())entry=cache.erase(entry);else ++entry;
            }
            auto& results=cache[{owned,owned.get()}];
            const auto [entry,inserted]=results.try_emplace(std::string(found->second->id()),nullptr);
            if(inserted){
                try{entry->second=std::make_shared<DeferredResult>();}catch(...){results.erase(entry);throw;}
                execute=true;
            }
            deferred=entry->second;
        }
        if(execute){
            try{deferred->promise.set_value(std::make_shared<const Result>(found->second->decode(*owned)));}
            catch(...){deferred->promise.set_exception(std::current_exception());}
        }
        return deferred->result.get();
    }
    SharedResult decodeAutomatic(const Flux::SharedRevolution& revolution){
        SharedResult selected;
        for(const auto& decoder:automatic){
            auto result=decode(decoder->id(),revolution);
            if(!selected||betterAutomatic(*result,*selected))selected=std::move(result);
        }
        if(!selected)throw MediaError(DiagnosticCode::RegistryNoAutomatic);
        return selected;
    }
    std::vector<std::shared_ptr<const Decoder>> ordered,automatic;
    std::map<std::string,std::shared_ptr<const Decoder>,std::less<>> byId;
    std::mutex gate;
    std::map<RevolutionKey,std::map<std::string,std::shared_ptr<DeferredResult>>,RevolutionKeyLess> cache;
};
Registry::Registry(std::vector<std::shared_ptr<const Decoder>> decoders):state_(std::make_shared<State>(std::move(decoders))){}
Registry::Registry():Registry(Composition::defaultDecoders()){}
const std::vector<std::shared_ptr<const Decoder>>& Registry::decoders() const noexcept{return state_->ordered;}
SharedResult Registry::decode(std::string_view id,const Flux::SharedRevolution& revolution) const{return state_->decode(id,revolution);}
SharedResult Registry::decodeAutomatic(const Flux::SharedRevolution& revolution) const{return state_->decodeAutomatic(revolution);}
std::optional<Selection> Registry::decodeBest(std::span<const Flux::SharedRevolution> revolutions,std::optional<std::string_view> decoderId) const {
    std::optional<Selection> selected;
    for(std::size_t index=0;index<revolutions.size();++index){
        auto result=decoderId?decode(*decoderId,revolutions[index]):decodeAutomatic(revolutions[index]);
        if(!selected||(decoderId?betterExplicit(*result,*selected->result):betterAutomatic(*result,*selected->result)))selected=Selection{index,std::move(result)};
    }
    return selected;
}
DecodeOperation Registry::operation() const {
    return [state=state_](auto id,const auto& revolution){return *state->decode(id,revolution);};
}
}
