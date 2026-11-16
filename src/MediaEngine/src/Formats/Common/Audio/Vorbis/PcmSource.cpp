#include "PcmSource.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <ivorbisfile.h>
#include <algorithm>
#include <bit>
#include <cstdio>
#include <exception>
#include <mutex>
#include <vector>
namespace RetroMediaLab::MediaEngine::Vorbis {
namespace {
using namespace Constants;
// The native decoder borrows this object. No C callback may unwind into C.
class Decoder final:public MediaFileSystems::RandomAccessData {
public:
    explicit Decoder(std::shared_ptr<const MediaFileSystems::RandomAccessData> source):source_(std::move(source)){}
    ~Decoder() override{if(opened_)ov_clear(&file_);}
    void open(std::stop_token token){
        token_=token;checkCancellation(token);
        if(!source_||source_->length()>std::uint64_t(MaxSigned))throw MediaError(DiagnosticCode::VorbisData);
        openNative();
        if(!ov_seekable(&file_)||ov_streams(&file_)<=0)throw MediaError(DiagnosticCode::VorbisData);
        const auto* first=ov_info(&file_,0);if(!first||first->channels<=0||first->rate<=0)throw MediaError(DiagnosticCode::VorbisData);
        channels_=first->channels;rate_=first->rate;frameBytes_=std::size_t(channels_)*SampleBytes;
        for(int i=0;i<ov_streams(&file_);++i){const auto* info=ov_info(&file_,i);if(!info||info->channels!=channels_||info->rate!=rate_)throw MediaError(DiagnosticCode::VorbisProfile);}
        const auto frames=ov_pcm_total(&file_,-1);checkFailure();
        if(frames<0||std::uint64_t(frames)>std::uint64_t(MaxSigned)/frameBytes_)throw MediaError(DiagnosticCode::VorbisData);
        length_=std::uint64_t(frames)*frameBytes_;scratch_.resize(std::max(DecodeBytes,frameBytes_));
    }
    std::uint64_t length()const noexcept override{return length_;}
    int channels()const noexcept{return channels_;}
    long rate()const noexcept{return rate_;}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{
        std::lock_guard lock(mutex_);token_=token;failure_=nullptr;checkCancellation(token);
        if(offset>length_||output.size()>length_-offset)throw MediaError(DiagnosticCode::VorbisRange);
        if(output.empty())return;
        try{
        if(!opened_)openNative();
        const auto frame=offset/frameBytes_;auto skip=std::size_t(offset%frameBytes_);
        const auto current=ov_pcm_tell(&file_);
        if(dirty_||current<0||std::uint64_t(current)!=frame){dirty_=true;const auto result=ov_pcm_seek(&file_,ogg_int64_t(frame));checkFailure();if(result<0)throw MediaError(DiagnosticCode::VorbisOperation,{std::to_string(result)});}
        dirty_=true;
        while(!output.empty()){
            checkCancellation(token);
            const auto frames=std::min(scratch_.size()/frameBytes_,(output.size()+skip+frameBytes_-1)/frameBytes_);
            const auto requested=frames*frameBytes_;int link=0;
            const auto count=ov_read(&file_,reinterpret_cast<char*>(scratch_.data()),int(requested),&link);checkFailure();
            if(count<=0||std::size_t(count)%frameBytes_)throw MediaError(DiagnosticCode::VorbisOperation,{std::to_string(count)});
            if constexpr(std::endian::native==std::endian::big){for(std::size_t i=0;i<std::size_t(count);i+=SampleBytes)std::swap(scratch_[i],scratch_[i+1]);}
            if(std::size_t(count)<=skip)throw MediaError(DiagnosticCode::VorbisData);
            const auto copied=std::min(output.size(),std::size_t(count)-skip);
            std::copy_n(scratch_.data()+skip,copied,output.data());output=output.subspan(copied);skip=0;
        }
        // A partial final frame was consumed: the next call will seek if needed.
        dirty_=false;
        }catch(...){if(opened_){ov_clear(&file_);opened_=false;}dirty_=true;throw;}
    }
private:
    void openNative()const{
        cursor_=0;const ov_callbacks callbacks{readCallback,seekCallback,nullptr,tellCallback};
        const auto result=ov_open_callbacks(const_cast<Decoder*>(this),&file_,nullptr,0,callbacks);
        opened_=result==0;checkFailure();
        if(result<0)throw MediaError(DiagnosticCode::VorbisOperation,{std::to_string(result)});
    }
    void checkFailure()const{if(failure_)std::rethrow_exception(failure_);checkCancellation(token_);}
    static std::size_t readCallback(void* data,std::size_t size,std::size_t count,void* context)noexcept{
        auto& self=*static_cast<Decoder*>(context);
        try{self.checkFailure();if(!size||!count)return 0;const auto available=self.source_->length()-self.cursor_;const auto items=std::min<std::uint64_t>(count,available/size);const auto bytes=std::size_t(items)*size;
            if(bytes)self.source_->readExactly(self.cursor_,std::span(static_cast<std::uint8_t*>(data),bytes),self.token_);self.cursor_+=bytes;return std::size_t(items);
        }catch(...){self.failure_=std::current_exception();return 0;}
    }
    static int seekCallback(void* context,ogg_int64_t offset,int origin)noexcept{
        auto& self=*static_cast<Decoder*>(context);
        try{self.checkFailure();std::uint64_t base=0;
            switch(origin){case SEEK_SET:break;case SEEK_CUR:base=self.cursor_;break;case SEEK_END:base=self.source_->length();break;default:return -1;}
            if(offset<0){const auto distance=std::uint64_t(-(offset+1))+1;if(distance>base)return -1;self.cursor_=base-distance;}
            else{const auto distance=std::uint64_t(offset);if(distance>self.source_->length()-base)return -1;self.cursor_=base+distance;}
            return 0;
        }catch(...){self.failure_=std::current_exception();return -1;}
    }
    static long tellCallback(void* context)noexcept{const auto& self=*static_cast<Decoder*>(context);return self.cursor_>std::uint64_t(MaxLong)?-1:long(self.cursor_);}
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;
    mutable OggVorbis_File file_{};
    mutable bool opened_=false;
    mutable std::uint64_t cursor_=0;
    mutable std::stop_token token_;
    mutable std::exception_ptr failure_;
    mutable std::mutex mutex_;
    mutable bool dirty_=true;
    mutable std::vector<std::uint8_t> scratch_;
    int channels_=0;long rate_=0;std::size_t frameBytes_=0;std::uint64_t length_=0;
};
}
PcmSource openPcmSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){auto decoder=std::make_shared<Decoder>(std::move(source));decoder->open(token);return {decoder,decoder->channels(),decoder->rate()};}
}
