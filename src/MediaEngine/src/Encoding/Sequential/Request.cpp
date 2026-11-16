#include "Request.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
namespace RetroMediaLab::MediaEngine {
SequentialEncodeRequest::SequentialEncodeRequest(std::string target,std::string machine,std::vector<SequentialDecodedBlock> blocks,std::vector<SequentialSegment> retained,std::map<std::string,std::string> parameters)
    :target_(std::move(target)),machine_(std::move(machine)),blocks_(std::move(blocks)),retained_(std::move(retained)),parameters_(std::move(parameters)) {
    if(QString::fromStdString(target_).trimmed().isEmpty()||QString::fromStdString(machine_).trimmed().isEmpty())throw MediaError(DiagnosticCode::SequentialEncodeRequest);
}
}
