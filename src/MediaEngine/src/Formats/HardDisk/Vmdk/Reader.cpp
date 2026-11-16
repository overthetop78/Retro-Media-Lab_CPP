#include "Descriptor.h"
#include "Sparse.h"
#include "../../../Images/ReadRejection.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vmdk {
namespace {
std::shared_ptr<const MediaDocument> readDocument(const RecognitionContext& context,bool includeRanges,std::stop_token token){checkCancellation(token);if(context.length()<Constants::SignatureSize)throw MediaError(DiagnosticCode::VmdkSource);const auto bytes=context.readAsync(0,Constants::SignatureSize,token).get();return std::equal(bytes.begin(),bytes.end(),Constants::SparseSignature.begin(),Constants::SparseSignature.end())?readSparse(context,includeRanges,token):readDescriptor(context,includeRanges,token);}
}
std::shared_ptr<const MediaDocument> createDocument(std::shared_ptr<const MediaSource> source,std::int64_t capacity,std::vector<DataRange> ranges,bool fixed,std::string descriptor){using namespace Constants;return std::make_shared<const MediaDocument>(std::move(source),std::string(FormatId),MediaKind::HardDisk,std::make_shared<const BlockImage>(capacity,SectorSize,std::move(ranges)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::map<std::string,std::string>{{std::string(VariantKey),std::string(fixed?FixedVariant:SparseVariant)},{std::string(DescriptorKey),std::move(descriptor)}});}
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{std::vector<std::uint8_t>(SparseSignature.begin(),SparseSignature.end()),std::vector<std::uint8_t>(DescriptorSignature.begin(),DescriptorSignature.end())},{std::string(Extension)},{MediaKind::HardDisk},{RepresentationKind::Blocks}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;try{readDocument(context,false,token);return true;}catch(...){if(isImageReadRejection(std::current_exception()))return false;throw;}});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{return readDocument(context,true,token);});}
}
