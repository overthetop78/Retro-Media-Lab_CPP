#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::WiiU {
bool wudLength(std::uint64_t value)noexcept{return value>=Constants::OpticalBytes&&value<=Constants::MaximumSigned&&value%Constants::OpticalBytes==0;}
std::optional<Header> parseHeader(std::span<const std::uint8_t> bytes,std::uint64_t sourceLength){using namespace Constants;if(bytes.size()<HeaderBytes||sourceLength<HeaderBytes||qFromLittleEndian<std::uint32_t>(bytes.data())!=Magic0||qFromLittleEndian<std::uint32_t>(bytes.data()+Magic1Offset)!=Magic1)return {};
    const auto chunk=qFromLittleEndian<std::uint32_t>(bytes.data()+ChunkOffset);const auto logical=qFromLittleEndian<std::uint64_t>(bytes.data()+LengthOffset);if(chunk<MinimumChunkBytes||chunk>=MaximumChunkBytes||!wudLength(logical))return {};const auto count=logical/chunk+(logical%chunk!=0);if(!count||count>MaximumIndexCount)return {};const auto indexEnd=HeaderBytes+count*IndexBytes;const auto array=indexEnd+(chunk-indexEnd%chunk)%chunk;if(array>sourceLength)return {};const auto stored=sourceLength-array;if(!stored||stored%chunk)return {};return Header{chunk,std::uint32_t(count),logical,array,stored/chunk};
}
std::optional<std::vector<std::uint32_t>> readIndex(const RecognitionContext& context,const Header& header,std::stop_token token){using namespace Constants;std::vector<std::uint32_t> result;result.reserve(header.indexCount);std::uint64_t done=0;while(done<header.indexCount){checkCancellation(token);const auto count=std::min<std::uint64_t>(IndexBufferBytes/IndexBytes,header.indexCount-done);const auto bytes=context.readAsync(HeaderBytes+std::int64_t(done*IndexBytes),int(count*IndexBytes),token).get();for(std::uint64_t i=0;i<count;++i){const auto value=qFromLittleEndian<std::uint32_t>(bytes.data()+i*IndexBytes);if(value>=header.storedCount)return {};result.push_back(value);}done+=count;}return result;}
namespace {
class WuxData final:public MediaFileSystems::RandomAccessData {
public:
    WuxData(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,Header header,std::vector<std::uint32_t> index):source_(std::move(source)),header_(header),index_(std::move(index)){
        if(!source_||!header_.chunkBytes||!header_.logicalBytes||index_.size()!=header_.indexCount||header_.arrayOffset>source_->length()||header_.storedCount>(source_->length()-header_.arrayOffset)/header_.chunkBytes||std::uint64_t(header_.indexCount)!=header_.logicalBytes/header_.chunkBytes+(header_.logicalBytes%header_.chunkBytes!=0))throw MediaError(DiagnosticCode::WuxIndex);
        for(const auto value:index_)if(value>=header_.storedCount)throw MediaError(DiagnosticCode::WuxIndex);
    }
    std::uint64_t length()const noexcept override{return header_.logicalBytes;}
    void readExactly(std::uint64_t offset,std::span<std::uint8_t> output,std::stop_token token)const override{checkCancellation(token);if(offset>length()||output.size()>length()-offset)throw MediaError(DiagnosticCode::WuxRange);while(!output.empty()){checkCancellation(token);const auto inside=offset%header_.chunkBytes;const auto count=std::min<std::uint64_t>(output.size(),header_.chunkBytes-inside);const auto stored=index_[std::size_t(offset/header_.chunkBytes)];source_->readExactly(header_.arrayOffset+std::uint64_t(stored)*header_.chunkBytes+inside,output.first(std::size_t(count)),token);offset+=count;output=output.subspan(std::size_t(count));}}
private:std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;Header header_;std::vector<std::uint32_t> index_;
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> wuxSource(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,Header header,std::vector<std::uint32_t> index){return std::make_shared<WuxData>(std::move(source),header,std::move(index));}
}
