#include <RetroMediaLab/MediaEngine/CuePreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Formats/Optical/BinCue/Sheet.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
struct CueDocument::Impl {BinCue::Sheet sheet;};
const CueDocument::Impl& CueDocument::require()const{if(!impl_)throw MediaError(DiagnosticCode::CueStructure);return *impl_;}
CueDocument CueDocument::parse(std::span<const std::string> lines,std::stop_token token){return CueDocument(std::make_shared<Impl>(Impl{BinCue::parseSheet(lines,token)}));}
CueDocument CueDocument::read(const std::filesystem::path& path,std::stop_token token){return CueDocument(std::make_shared<Impl>(Impl{BinCue::readSheet(path,token)}));}
std::vector<std::string> CueDocument::declaredFiles()const{std::vector<std::string> result;for(const auto& file:require().sheet.files())result.push_back(file.path());return result;}
bool CueDocument::isSingleBinaryRawAudio(int maximumTracks)const{
    const auto files=require().sheet.files();if(files.size()!=1||files.front().kind()!=BinCue::FileKind::Binary)return false;
    const auto tracks=files.front().tracks();if(tracks.empty()||maximumTracks<0||tracks.size()>static_cast<std::size_t>(maximumTracks))return false;
    for(std::size_t i=0;i<tracks.size();++i){const auto& fields=tracks[i].fields();
        if(fields.number!=static_cast<std::int64_t>(i)+1||(fields.mode!=OpticalTrackMode::Mode1Raw2352&&fields.mode!=OpticalTrackMode::Audio)
            ||!std::any_of(fields.indexes.begin(),fields.indexes.end(),[](const auto& index){return index.number()==1;}))return false;}
    return true;
}
CueDocument CueDocument::withSingleFile(std::string path)const{
    const auto& sheet=require().sheet;const auto& first=sheet.files().front();
    return CueDocument(std::make_shared<Impl>(Impl{BinCue::Sheet({BinCue::FileDeclaration(std::move(path),first.kind(),{first.tracks().begin(),first.tracks().end()})},sheet.catalog(),sheet.metadata())}));
}
void CueDocument::write(std::ostream& output,std::stop_token token)const{
    const auto text=BinCue::serializeSheet(require().sheet,token);checkCancellation(token);
    try {output.write(text.data(),static_cast<std::streamsize>(text.size()));}
    catch(const std::ios_base::failure&){throw MediaError(DiagnosticCode::AtomicWrite,{std::string(BinCue::Constants::FormatId)});}
    if(!output)throw MediaError(DiagnosticCode::AtomicWrite,{std::string(BinCue::Constants::FormatId)});
    checkCancellation(token);
}
}
