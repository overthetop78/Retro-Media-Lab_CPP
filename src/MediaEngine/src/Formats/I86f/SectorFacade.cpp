#include <RetroMediaLab/MediaEngine/I86fReconstruction.h>
#include "SectorReader.h"
#include "../../Decoding/Registry.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
std::future<std::shared_ptr<const MediaDocument>> reconstructI86fSectorsAsync(std::filesystem::path path,std::stop_token token){return std::async(std::launch::async,[path=std::move(path),token]{checkCancellation(token);const auto text=path.u8string();SourceOptions options;options.primaryPath=std::string(text.begin(),text.end());if(Primitives::isBlankIdentifier(options.primaryPath))throw MediaError(DiagnosticCode::DiskPreparationRequest);auto source=std::make_shared<MediaSource>(std::move(options));RecognitionContext context(source);const auto bytes=context.readBytesAsync(token).get();auto image=I86f::read(bytes,token);auto sectors=I86f::reconstructSectors(image,Decoding::Registry{}.operation(),token);return floppySectorDocument(std::move(source),std::move(sectors));});}
}
