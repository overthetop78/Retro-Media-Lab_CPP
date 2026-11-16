#pragma once
#include "DiskExplorationTypes.h"
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace RetroMediaLab::MediaEngine {
// Transport-only snapshot: device providers decide the stage and track states.
// Translation remains deferred; an external tool message is preserved verbatim.
struct PhysicalTrackReadingState {
    int cylinder=0,head=0;
    std::string state;
    int attempts=0;
};
struct PhysicalReadingState {
    std::string stage;
    int completedTracks=0,totalTracks=0;
    std::optional<int> cylinder,head;
    int attempt=0;
    std::shared_ptr<const DiskTrackInfo> acquiredTrack;
    std::vector<PhysicalTrackReadingState> trackStates;
    std::optional<std::string> messageCode;
    std::map<std::string,std::string> messageParameters;
    std::optional<std::string> externalMessage;
};
}
