#include <RetroMediaLab/MediaEngine/OpticalDiscSession.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../../Composition/Recognition.h"
#include "CompanionAudio.h"
namespace RetroMediaLab::MediaEngine {
namespace {
struct OpeningOwnership {
 std::shared_ptr<const MediaDocument>& document;bool transferred=false;
 ~OpeningOwnership(){if(transferred)return;auto drive=document?document->mediaSource()->options().drive:nullptr;document.reset();if(drive)try{drive->reset();}catch(...){}}
};
}
std::future<std::unique_ptr<OpticalDiscSession>> OpticalDiscSession::openAsync(std::filesystem::path path,std::stop_token token,bool companions){return std::async(std::launch::async,[path=std::move(path),token,companions]{
 checkCancellation(token);SourceOptions options;const auto text=path.u8string();options.primaryPath=std::string(text.begin(),text.end());options.trackDriveAccesses=false;
 auto document=MediaRecognitionComposition::createDefault().readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();OpeningOwnership ownership{document};checkCancellation(token);
 if(companions)document=attachOpticalCompanionAudio(document,token);checkCancellation(token);
 std::unique_ptr<OpticalDiscSession> session(new OpticalDiscSession(document,false));ownership.transferred=true;return session;
 });}
}
