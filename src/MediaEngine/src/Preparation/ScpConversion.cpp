#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Reconstruction/Iso/Reader.h"
#include "../Reconstruction/Iso/Automatic.h"
#include "../Reconstruction/Iso/RuntimeConstants.h"
#include "../Reconstruction/AmigaReader.h"
#include "../Decoding/Registry.h"
#include "../Formats/Atr/Atr.h"
#include "../Formats/St/Writer.h"
#include "../Formats/Adf/AdfImages.h"
#include "../Formats/Adf/Constants.h"
#include "../Formats/FormatIds.h"
#include "../Sources/RandomAccessSources.h"
#include "../Primitives/Identifiers.h"
namespace RetroMediaLab::MediaEngine {
namespace {
struct ScpPreparationReaders {Decoding::Registry decoders;IsoReconstruction::Reader iso{decoders.operation()};};
ScpPreparationReaders& readers(){static ScpPreparationReaders result;return result;}
}
std::future<void> convertScpAsync(std::filesystem::path source,std::filesystem::path output,std::string extension,std::stop_token token){
    return std::async(std::launch::async,[source=std::move(source),output=std::move(output),extension=std::move(extension),token]{
        namespace C=IsoReconstruction::RuntimeConstants;namespace M=MediaFileSystems;
        const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
        checkCancellation(token);const bool atr=same(extension,C::AtrExtension),st=same(extension,C::StExtension),adf=same(extension,Adf::Constants::AdfExtension);
        if(!atr&&!st&&!adf)throw MediaError(DiagnosticCode::ScpPreparationTarget,{extension});
        std::error_code identityError;if(std::filesystem::equivalent(source,output,identityError))throw MediaError(DiagnosticCode::AtomicSetExists,{output.string()});
        std::shared_ptr<const M::SectorImage> image;
        auto& services=readers();
        if(adf){
            if(same(source.extension().string(),Scp::Constants::Extension))image=AmigaReconstruction::read(Scp::read(*fileSource(source),token),services.decoders.operation(),token);
            else image=Adf::read(fileSource(source),{},token);
        }else{
            image=services.iso.readAtari(source,{}, {},token);
        }
        const auto& format=image->geometry().formatId;
        const bool matches=atr?Primitives::asciiIdentifierStartsWithIgnoreCase(format,Formats::AtariPrefix):st?Primitives::asciiIdentifierStartsWithIgnoreCase(format,Formats::AtariStPrefix):same(format,Formats::AmigaDos)||same(format,Formats::AmigaDosHighDensity);
        if(!matches)throw MediaError(DiagnosticCode::ScpPreparationFormat,{format,extension});
        checkCancellation(token);
        if(atr)Atr::write(*image,output,format,token);else if(st)St::write(*image,output,token);else Adf::write(*image,output,token);
    });
}
}
