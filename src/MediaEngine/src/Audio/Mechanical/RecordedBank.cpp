#include "RecordedBank.h"
#include "RecordingDecoder.h"
#include <QCoreApplication>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
namespace {
std::filesystem::path samplePath(const RecordedSample& sample){return std::filesystem::path(QString::fromUtf8(sample.filePath.data(),static_cast<qsizetype>(sample.filePath.size())).toStdU16String());}
}
std::shared_ptr<RecordedRenderer> sharedRecordedBank(){static const std::shared_ptr<RecordedRenderer> bank=createRecordedBank();return bank;}
std::unique_ptr<RecordedRenderer> createRecordedBank(std::filesystem::path root,std::filesystem::path executable){
    if(root.empty())root=std::filesystem::path(QCoreApplication::applicationDirPath().toStdU16String())/Constants::AssetsDirectory/Constants::BankDirectory;
    const auto profiles=recordedProfiles();
    return std::make_unique<RecordedRenderer>(std::vector<RecordedProfile>(profiles.begin(),profiles.end()),
        [root,executable](const RecordedSample& sample,int rate){return decodeRecording(root/samplePath(sample),rate,executable).samples;},
        [root,executable](const RecordedSample& sample){const auto decoded=decodeRecording(root/samplePath(sample),0,executable);return static_cast<double>(decoded.samples.size())/decoded.sampleRate;});
}
}
