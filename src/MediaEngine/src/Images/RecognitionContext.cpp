#include "Readers.h"
#include "../Primitives/Identifiers.h"
#include "../Primitives/Path.h"
#include "../Sources/RandomAccessSources.h"
#include "../Sources/DriveAccess.h"
#include <QString>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <mutex>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
struct SourceAccess {
    std::string path;std::mutex mutex;std::shared_ptr<const M::RandomAccessData> data;
    std::shared_ptr<const M::RandomAccessData> get(){std::lock_guard lock(mutex);if(!data){std::u8string value;value.reserve(path.size());for(const auto byte:path)value.push_back(char8_t(byte));data=fileSource(std::filesystem::path(value));}return data;}
};
}
struct RecognitionContext::Impl {
    std::shared_ptr<const MediaSource> source;
    std::shared_ptr<SourceAccess> access;
    std::int64_t length=0;std::int32_t pageSize;std::string extension;
    bool provided;DriveFactory drives;std::optional<MediaKind> driveKind;
    mutable std::mutex sourceMutex,driveMutex,pagesMutex;
    std::map<std::int64_t,std::shared_future<std::vector<std::uint8_t>>> pages;
    Impl(std::shared_ptr<const MediaSource> value,std::int32_t size,std::shared_ptr<const M::RandomAccessData> data,DriveFactory factory):source(std::move(value)),access(std::make_shared<SourceAccess>()),pageSize(size),provided(source&&bool(source->options().drive)),drives(std::move(factory)){
        if(!source)throw M::MediaError(M::DiagnosticCode::ContextSourceNull);
        if(QString::fromUtf8(source->primaryPath()).trimmed().isEmpty())throw M::MediaError(M::DiagnosticCode::ContextSourcePath);
        if(size<=0)throw M::MediaError(M::DiagnosticCode::ContextPageSize);
        access->path=source->primaryPath();access->data=std::move(data);
        if(source->knownLength())length=*source->knownLength();else{const auto count=access->get()->length();if(count>std::uint64_t(std::numeric_limits<std::int64_t>::max()))throw M::MediaError(M::DiagnosticCode::ContextLengthOverflow);length=std::int64_t(count);}
        if(length<0)throw M::MediaError(M::DiagnosticCode::ContextKnownLength);
        extension=Primitives::utf8Identifier(Primitives::pathExtensionUtf8(source->primaryPath())).toLower().toStdString();
    }
    std::shared_ptr<const MediaSource> snapshot()const{std::lock_guard lock(sourceMutex);return source;}
    std::shared_future<std::vector<std::uint8_t>> page(std::int64_t index,std::stop_token token){
        std::lock_guard lock(pagesMutex);if(const auto found=pages.find(index);found!=pages.end())return found->second;
        const auto current=snapshot();const auto drive=provided?std::shared_ptr<DriveMechanics>{}:current->options().drive;
        const auto offset=index*pageSize;const auto count=std::size_t(std::min(std::int64_t(pageSize),length-offset));
        auto future=std::async(std::launch::deferred,[access=access,drive,offset,count,token]{
            M::checkCancellation(token);std::vector<std::uint8_t> bytes(count);
            DriveAccessScope scope(drive.get(),drive?drive->state().position:DrivePosition{},false);
            const auto data=access->get();
            if(std::uint64_t(offset)>data->length()||count>data->length()-std::uint64_t(offset))throw M::MediaError(M::DiagnosticCode::ContextPageEnded,{std::to_string(std::max(std::uint64_t(offset),data->length()))});
            data->readExactly(std::uint64_t(offset),bytes,token);scope.finish();return bytes;
        }).share();pages.emplace(index,future);return future;
    }
    std::vector<std::uint8_t> read(std::int64_t offset,std::int32_t count,std::stop_token token){
        if(offset<0)throw M::MediaError(M::DiagnosticCode::ContextReadOffset);if(count<0)throw M::MediaError(M::DiagnosticCode::ContextReadCount);
        if(offset>length||count>length-offset)throw M::MediaError(M::DiagnosticCode::ContextReadRange);if(!count)return {};
        std::vector<std::uint8_t> destination(std::size_t(count),std::uint8_t{});
        const auto first=offset/pageSize,last=(offset+count-1)/pageSize;
        for(auto index=first;index<=last;++index){M::checkCancellation(token);const auto future=page(index,token);const auto& bytes=future.get();const auto start=index*pageSize;const auto sourceOffset=std::size_t(std::max(std::int64_t(0),offset-start)),destinationOffset=std::size_t(std::max(std::int64_t(0),start-offset));const auto copied=std::min(bytes.size()-sourceOffset,destination.size()-destinationOffset);std::copy_n(bytes.begin()+std::ptrdiff_t(sourceOffset),copied,destination.begin()+std::ptrdiff_t(destinationOffset));}
        return destination;
    }
};
RecognitionContext::RecognitionContext(std::shared_ptr<const MediaSource> source,std::int32_t size,std::shared_ptr<const M::RandomAccessData> data,DriveFactory factory):impl_(std::make_shared<Impl>(std::move(source),size,std::move(data),std::move(factory))){}
std::shared_ptr<const MediaSource> RecognitionContext::source()const{return impl_->snapshot();}
std::shared_ptr<const M::RandomAccessData> RecognitionContext::dataSource()const{return impl_->access->get();}
std::int64_t RecognitionContext::length()const noexcept{return impl_->length;}
const std::string& RecognitionContext::extension()const noexcept{return impl_->extension;}
std::optional<std::string> RecognitionContext::requestedFormatId()const{return source()->options().requestedFormatId;}
bool RecognitionContext::ownsDrive()const noexcept{return !impl_->provided;}
void RecognitionContext::ensureDrive(MediaKind kind)const{
    std::lock_guard lock(impl_->driveMutex);const auto current=impl_->snapshot();if(!impl_->drives||(current->options().drive&&(impl_->provided||impl_->driveKind==kind)))return;
    if(current->options().drive)current->options().drive->reset();auto drive=impl_->drives(kind);impl_->driveKind=kind;
    {std::lock_guard sourceLock(impl_->sourceMutex);impl_->source=current->withDrive(drive);}
    if(drive)try{drive->insert();}catch(...){try{drive->reset();}catch(...){}throw;}
}
std::future<std::vector<std::uint8_t>> RecognitionContext::readAsync(std::int64_t offset,std::int32_t count,std::stop_token token)const{return std::async(std::launch::deferred,[impl=impl_,offset,count,token]{return impl->read(offset,count,token);});}
std::future<std::vector<std::uint8_t>> RecognitionContext::readHeaderAsync(std::int32_t count,std::stop_token token)const{return std::async(std::launch::deferred,[impl=impl_,count,token]{if(count<0)throw M::MediaError(M::DiagnosticCode::ContextReadCount);return impl->read(0,std::int32_t(std::min(std::int64_t(count),impl->length)),token);});}
std::future<std::vector<std::uint8_t>> RecognitionContext::readBytesAsync(std::stop_token token)const{return std::async(std::launch::deferred,[impl=impl_,token]{if(impl->length>std::numeric_limits<std::int32_t>::max())throw M::MediaError(M::DiagnosticCode::ContextFullRead);return impl->read(0,std::int32_t(impl->length),token);});}
bool sameIdentifier(std::string_view left,std::string_view right)noexcept{return Primitives::asciiIdentifierEqualsIgnoreCase(left,right);}

bool ImageReader::supportsFormatId(std::string_view id)const{const auto& ids=capabilities().formatIds;return std::any_of(ids.begin(),ids.end(),[&](const auto& value){return sameIdentifier(value,id);});}
}
