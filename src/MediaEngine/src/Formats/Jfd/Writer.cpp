#include "Adapter.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
namespace RetroMediaLab::MediaEngine::Jfd {
std::shared_ptr<const ImageWriter> writerAdapter() {
    using namespace Constants;
    return sectorWriterAdapter(std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},
        [](const auto& image,const MediaDocument& document,auto,auto){return document.mediaKind()==MediaKind::Floppy&&(document.metadata().contains(std::string(NativeKey))||canCreate(image));},
        [](const auto& image,const MediaDocument& document,const auto& path,auto,std::stop_token token){
            const auto native=restore(document.metadata(),token);const auto bytes=native?rewrite(*native,image,token):create(image,token);
            const auto flag=document.metadata().find(std::string(GzipKey));const bool gzip=flag==document.metadata().end()||flag->second!=FalseText;
            writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token writeToken){if(gzip)writeGzip(bytes,output,writeToken);else output(bytes);},token);
            return std::vector<std::filesystem::path>{path};
        });
}
}
