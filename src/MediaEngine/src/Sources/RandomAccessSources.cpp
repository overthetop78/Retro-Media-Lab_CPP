#include "RandomAccessSources.h"
#include "SourceConstants.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <vector>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace MediaFileSystems;
std::string utf8(const std::filesystem::path& path) { const auto value = path.u8string(); return std::string(value.begin(), value.end()); }
class MemorySource final : public RandomAccessData {
public:
    explicit MemorySource(Bytes data) : bytes_(data.begin(), data.end()) {}
    std::uint64_t length() const noexcept override { return bytes_.size(); }
    void readExactly(std::uint64_t offset, MutableBytes destination, std::stop_token token) const override {
        checkCancellation(token);
        if (offset > length() || destination.size() > length() - offset) throw MediaError(DiagnosticCode::ReadOutOfBounds);
        if (destination.empty()) return;
        std::copy_n(bytes_.begin() + std::size_t(offset), destination.size(), destination.begin());
    }
private:
    const std::vector<std::uint8_t> bytes_;
};
class FileSource final : public RandomAccessData {
public:
    explicit FileSource(const std::filesystem::path& path) {
        std::error_code error;
        path_ = std::filesystem::absolute(path, error);
        if (error) throw MediaError(DiagnosticCode::OpenFailed, {utf8(path)});
        length_ = std::filesystem::file_size(path_, error);
        if (error) throw MediaError(DiagnosticCode::SourceLengthFailed, {utf8(path_)});
    }
    std::uint64_t length() const noexcept override { return length_; }
    void readExactly(std::uint64_t offset, MutableBytes destination, std::stop_token token) const override {
        checkCancellation(token);
        if (offset > length_ || destination.size() > length_ - offset) throw MediaError(DiagnosticCode::ReadOutOfBounds);
        if (destination.empty()) return;
        std::ifstream stream(path_, std::ios::binary);
        if (!stream) throw MediaError(DiagnosticCode::OpenFailed, {utf8(path_)});
        if (offset > std::uint64_t(std::numeric_limits<std::streamoff>::max())) throw MediaError(DiagnosticCode::ReadOutOfBounds);
        stream.seekg(std::streamoff(offset));
        if (!stream) throw MediaError(DiagnosticCode::SourceSeekFailed, {std::to_string(offset)});
        std::size_t done = 0;
        while (done < destination.size()) {
            checkCancellation(token);
            const auto count = std::min(SourceConstants::ReadChunk, destination.size() - done);
            stream.read(reinterpret_cast<char*>(destination.data() + done), std::streamsize(count));
            const auto received = std::size_t(stream.gcount());
            done += received;
            if (stream.bad()) throw MediaError(DiagnosticCode::SourceReadFailed, {std::to_string(offset + done)});
            if (received != count) throw MediaError(DiagnosticCode::SourceEnded, {std::to_string(offset + done)});
        }
    }
private:
    std::filesystem::path path_;
    std::uint64_t length_ = 0;
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> memorySource(MediaFileSystems::Bytes bytes) { return std::make_shared<MemorySource>(bytes); }
std::shared_ptr<const MediaFileSystems::RandomAccessData> fileSource(const std::filesystem::path& path) { return std::make_shared<FileSource>(path); }
}
