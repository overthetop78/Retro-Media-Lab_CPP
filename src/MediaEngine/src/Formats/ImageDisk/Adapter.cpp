#include "Adapter.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "ImageDisk.h"
#include "../FormatIds.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ImageDisk {
namespace F=MediaFileSystems::Formats;
const ReaderCapabilities& Reader::capabilities()const {
    static const ReaderCapabilities result{
        {std::string(Formats::Imd),std::string(F::EpsonQx10_320),std::string(F::EpsonQx10_396),std::string(F::EpsonQx10_399),std::string(F::EpsonQx10_400),std::string(F::EpsonQx10Booter),std::string(F::EpsonQx10Logo)},
        {std::string(Constants::Extension)},{{Constants::Signature.begin(),Constants::Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};
    return result;
}
bool Reader::supportsFormatId(std::string_view id)const {
    return ImageReader::supportsFormatId(id)||(id.size()>=F::EpsonQx10Prefix.size()&&Primitives::ordinalIdentifierEqualsIgnoreCase(id.substr(0,F::EpsonQx10Prefix.size()),F::EpsonQx10Prefix));
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
        if(context.length()<Constants::SignatureSize)return false;
        const auto header=context.readHeaderAsync(Constants::SignatureSize,token).get();
        return std::equal(header.begin(),header.end(),Constants::Signature.begin(),Constants::Signature.end());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto bytes=context.readBytesAsync(token).get();const auto image=readDetailed(bytes,token);
        return floppySectorDocument(context.source(),image.sectorImage,metadata(image));
    });
}
}
