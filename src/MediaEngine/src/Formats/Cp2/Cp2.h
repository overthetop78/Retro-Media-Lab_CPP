#pragma once
#include "../../Images/Readers.h"
#include <span>
namespace RetroMediaLab::MediaEngine::Cp2 {
struct SectorDescriptor {std::uint8_t cylinder{},head{},number{};std::uint32_t size{};std::uint16_t position{};};
std::vector<SectorDescriptor> parseTrack(std::span<const std::uint8_t>);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(std::span<const std::uint8_t>,std::stop_token={});
class Reader final:public ImageReader {
public:
    RETROMEDIALAB_MEDIAENGINE_API const ReaderCapabilities& capabilities()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool supportsFormatId(std::string_view)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<bool> canReadAsync(RecognitionContext,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const MediaDocument>> readAsync(RecognitionContext,std::stop_token={})const override;
};
}
