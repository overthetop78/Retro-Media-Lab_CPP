#pragma once
#include "DiskExplorationTypes.h"
#include "Export.h"
#include <filesystem>
#include <future>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
class ExploredDisk final {
public:
 RETROMEDIALAB_MEDIAENGINE_API const DiskImageInfo& info()const noexcept;
 RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const ExploredDisk> selectFormat(std::string_view)const;
 ExploredDisk(const ExploredDisk&)=delete;
 ExploredDisk& operator=(const ExploredDisk&)=delete;
private:
 struct State;std::shared_ptr<State> state_;
 explicit ExploredDisk(std::shared_ptr<State> state):state_(std::move(state)){}
 friend class DiskProjectionAccess;
};
RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const ExploredDisk>> exploreDiskAsync(std::filesystem::path,std::optional<std::string> format={},DiskExplorationProgressCallback={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const ExploredDisk>> exploreDiskAsync(std::shared_ptr<const MediaDocument>,std::optional<std::string> format={},DiskExplorationProgressCallback={},std::stop_token={});
}
