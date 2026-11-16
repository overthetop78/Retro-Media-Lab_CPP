#pragma once
#include "Constants.h"
#include "../../../Images/Readers.h"
#include "../../../Images/Writers.h"
#include "../../../Models/Blocks/BlockImage.h"
namespace RetroMediaLab::MediaEngine::Vhd {
struct Footer {std::uint64_t dataOffset;std::int64_t currentSize;std::uint16_t cylinders;std::uint8_t heads,sectors;std::uint32_t diskType;std::string uuid;};
std::uint32_t checksum(std::span<const std::uint8_t>,std::size_t offset);
Footer readFooter(const RecognitionContext&,std::stop_token);
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
class Writer final:public ImageWriter {
public:
    RETROMEDIALAB_MEDIAENGINE_API const WriterCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canWrite(const MediaDocument&,std::string_view,std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> writeAsync(std::shared_ptr<const MediaDocument>,std::filesystem::path,std::string,std::stop_token={})const override;
};
}
