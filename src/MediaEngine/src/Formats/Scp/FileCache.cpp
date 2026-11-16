#include "FileCache.h"
#include "../../Sources/FileIdentity.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace RetroMediaLab::MediaEngine::Scp {
namespace {
FileIdentity identity(const std::filesystem::path& path){auto result=fileIdentity(path);if(!result.exists){const auto encoded=path.u8string();throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{std::string(encoded.begin(),encoded.end())});}return result;}
struct Entry {
    explicit Entry(FileIdentity value):file(std::move(value)){}
    FileIdentity file;
    std::mutex gate;
    std::condition_variable_any changed;
    bool ready=false;
    SharedImage image;
    std::exception_ptr error;
};
struct Worker {std::shared_ptr<std::atomic_bool> finished;std::jthread thread;};
}
struct FileCache::State {
    std::mutex gate;
    std::unordered_map<std::string,std::shared_ptr<Entry>> entries;
    // Destroyed before entries and gate; jthread requests shutdown and joins every owned load.
    std::list<Worker> workers;
    ~State(){
        for(auto& worker:workers)worker.thread.request_stop();
        // A caller-supplied loader may itself own a cache copy. Its final capture is destroyed
        // on the finished worker; release that thread handle instead of attempting a self-join.
        for(auto& worker:workers)if(worker.thread.get_id()==std::this_thread::get_id())worker.thread.detach();
    }
    std::shared_ptr<Entry> obtain(const FileIdentity& file,Loader loader){
        // Joining a finished worker may destroy a caller-supplied loader. Do it after unlocking.
        std::list<Worker> retired;
        std::unique_lock lock(gate);
        for(auto it=workers.begin();it!=workers.end();){const auto current=it++;if(current->finished->load(std::memory_order_acquire))retired.splice(retired.end(),workers,current);}
        const auto found=entries.find(file.key);if(found!=entries.end()&&found->second->file==file)return found->second;
        auto entry=std::make_shared<Entry>(file);entries[file.key]=entry;
        try{
            workers.emplace_back();auto& worker=workers.back();
            try{
                worker.finished=std::make_shared<std::atomic_bool>(false);
                worker.thread=std::jthread([entry,loader=std::move(loader),finished=worker.finished](std::stop_token lifetime){
                    SharedImage image;std::exception_ptr error;
                    try{image=loader(entry->file.path,lifetime);if(!image)throw MediaError(DiagnosticCode::ScpArgument,{std::string(Constants::ImageArgument)});}catch(...){error=std::current_exception();}
                    {std::lock_guard entryLock(entry->gate);entry->image=std::move(image);entry->error=error;entry->ready=true;}
                    entry->changed.notify_all();finished->store(true,std::memory_order_release);
                });
            }catch(...){workers.pop_back();throw;}
        }catch(...){entries.erase(file.key);throw;}
        return entry;
    }
    void evict(const std::shared_ptr<Entry>& entry){std::lock_guard lock(gate);const auto found=entries.find(entry->file.key);if(found!=entries.end()&&found->second==entry)entries.erase(found);}
};
FileCache::FileCache():state_(std::make_shared<State>()){}
void FileCache::remember(const std::filesystem::path& path,SharedImage image)const{
    if(!image)throw MediaError(DiagnosticCode::ScpArgument,{std::string(Constants::ImageArgument)});
    auto entry=std::make_shared<Entry>(identity(path));entry->image=std::move(image);entry->ready=true;
    {std::lock_guard lock(state_->gate);state_->entries[entry->file.key].swap(entry);}
}
std::future<SharedImage> FileCache::getOrAddAsync(std::filesystem::path path,Loader loader,std::stop_token token)const{
    return std::async(std::launch::deferred,[state=state_,path=std::move(path),loader=std::move(loader),token]{
        if(!loader)throw MediaError(DiagnosticCode::ScpArgument,{std::string(Constants::LoaderArgument)});
        const auto entry=state->obtain(identity(path),loader);std::unique_lock lock(entry->gate);
        if(!entry->changed.wait(lock,token,[&]{return entry->ready;}))MediaFileSystems::checkCancellation(token);
        const auto error=entry->error;const auto image=entry->image;lock.unlock();
        if(error){state->evict(entry);std::rethrow_exception(error);}return image;
    });
}
}
