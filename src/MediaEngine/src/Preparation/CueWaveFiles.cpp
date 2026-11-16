#include <RetroMediaLab/MediaEngine/CuePreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Formats/Optical/BinCue/Sheet.h"
#include "../Formats/Optical/BinCue/WaveSource.h"
#include "../Sources/RandomAccessSources.h"
#include "../Sources/AtomicImageSetWriter.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
std::filesystem::path nativePath(std::string_view value){return std::filesystem::path(Primitives::utf8Identifier(value).toStdU16String());}
std::string utf8Path(const std::filesystem::path& value){return QString::fromStdU16String(value.u16string()).toUtf8().toStdString();}
bool samePath(const std::filesystem::path& a,const std::filesystem::path& b){
    std::error_code error;if(std::filesystem::equivalent(a,b,error))return true;
    return Primitives::ordinalIdentifierEqualsIgnoreCase(utf8Path(std::filesystem::absolute(a).lexically_normal()),utf8Path(std::filesystem::absolute(b).lexically_normal()));
}
void prepare(const std::filesystem::path& sourceCue,const std::filesystem::path& stagedCue,const CueStageFile& stageFile,int copyBufferBytes,std::stop_token token,const CueUnsupported& unsupported){
    checkCancellation(token);if(copyBufferBytes<=0||!stageFile)throw MediaError(DiagnosticCode::CueStructure);
    const auto sheet=BinCue::readSheet(sourceCue,token);
    const auto sourceDirectory=std::filesystem::absolute(sourceCue).parent_path(),stagedDirectory=std::filesystem::absolute(stagedCue).parent_path();
    std::vector<std::filesystem::path> originals;originals.push_back(std::filesystem::absolute(sourceCue));
    for(const auto& file:sheet.files())originals.push_back(std::filesystem::absolute(sourceDirectory/nativePath(file.path())).lexically_normal());
    std::vector<BinCue::FileDeclaration> files;
    std::vector<ImageFileOutput> outputs;
    const auto protectSource=[&](const auto& path){if(std::any_of(originals.begin(),originals.end(),[&](const auto& source){return samePath(source,path);}))throw MediaError(DiagnosticCode::AtomicSetDuplicate);};
    protectSource(stagedCue);
    for(std::size_t i=0;i<sheet.files().size();++i){
        checkCancellation(token);const auto& file=sheet.files()[i];const auto stagedPath=stageFile(originals[i+1]);if(stagedPath.empty())throw MediaError(DiagnosticCode::CueStructure);
        const auto staged=std::filesystem::absolute(stagedPath).lexically_normal();
        protectSource(staged);if(samePath(staged,stagedCue))throw MediaError(DiagnosticCode::AtomicSetDuplicate);checkCancellation(token);
        const auto converted=std::any_of(outputs.begin(),outputs.end(),[&](const auto& output){return samePath(output.path,staged);});
        if(file.kind()==BinCue::FileKind::WavePcm){
            if(std::any_of(file.tracks().begin(),file.tracks().end(),[](const auto& track){return track.fields().mode!=OpticalTrackMode::Audio;})){
                if(unsupported)if(const auto error=unsupported())std::rethrow_exception(error);
                throw MediaError(DiagnosticCode::CueStructure);
            }
        }
        if(file.kind()==BinCue::FileKind::WavePcm&&!converted){
            const auto pcm=BinCue::wavePcmSource(fileSource(originals[i+1]),false,token);
            outputs.push_back({staged,[pcm,copyBufferBytes](const ImageOutput& write,std::stop_token stop){
                std::vector<std::uint8_t> buffer(static_cast<std::size_t>(std::min<std::uint64_t>(static_cast<std::uint64_t>(copyBufferBytes),pcm->length())));
                for(std::uint64_t offset=0;offset<pcm->length();){checkCancellation(stop);const auto size=static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(),pcm->length()-offset));
                    pcm->readExactly(offset,std::span(buffer).first(size),stop);write(std::span(buffer).first(size));offset+=size;}
                const auto padding=(BinCue::Constants::RawBytes-pcm->length()%BinCue::Constants::RawBytes)%BinCue::Constants::RawBytes;
                if(padding){const std::vector<std::uint8_t> zeros(static_cast<std::size_t>(padding));write(zeros);}
            }});
        }
        auto relative=staged.lexically_relative(stagedDirectory);if(relative.empty())relative=staged;
        files.emplace_back(utf8Path(relative),file.kind()==BinCue::FileKind::WavePcm?BinCue::FileKind::Binary:file.kind(),std::vector<BinCue::TrackDeclaration>(file.tracks().begin(),file.tracks().end()));
    }
    const auto text=BinCue::serializeSheet(BinCue::Sheet(std::move(files),sheet.catalog(),sheet.metadata()),token);
    outputs.push_back({stagedCue,[text](const ImageOutput& write,std::stop_token stop){checkCancellation(stop);write({reinterpret_cast<const std::uint8_t*>(text.data()),text.size()});}});
    writeAtomicImageSet(outputs,token);
}
}
std::future<void> prepareCueWaveFilesAsync(std::filesystem::path source,std::filesystem::path staged,CueStageFile stageFile,std::int32_t bufferBytes,std::stop_token token,CueUnsupported unsupported){
    return std::async(std::launch::async,[source=std::move(source),staged=std::move(staged),stageFile=std::move(stageFile),bufferBytes,token,unsupported=std::move(unsupported)]{
        try{prepare(source,staged,stageFile,bufferBytes,token,unsupported);}
        catch(const std::filesystem::filesystem_error& error){throw MediaError(DiagnosticCode::AtomicOpen,{utf8Path(error.path1())});}
    });
}
}
