#pragma once
#include "InfrastructureApi.h"
#include <QString>
#include <vector>
#include <map>
#include <optional>
#include <set>
namespace RetroMediaLab::Infrastructure {
enum class GwTrackState {Pending,Active,Success,Retry,Failed};
struct GwTrackProgress {
    std::vector<int> cylinders,heads;
    std::map<std::pair<int,int>,GwTrackState> states;
    std::optional<int> cylinder,head,nextCylinder,nextHead,total,totalOnHead;
    std::map<int,int> completedPerHead;
    std::map<int,std::vector<int>> cylindersPerHead;
    int completed=0,completedOnHead=0;
};
class RETROMEDIALAB_INFRASTRUCTURE_API GwProgressTracker final {
public:
    void reset();
    void acceptChunk(const QString&,bool finished=false);
    const GwTrackProgress& progress()const noexcept{return progress_;}
private:
    void acceptLine(const QString&);
    GwTrackProgress progress_;
    QString pending_;
    std::set<std::pair<int,int>> completed_;
    bool discarding_=false;
};
}
