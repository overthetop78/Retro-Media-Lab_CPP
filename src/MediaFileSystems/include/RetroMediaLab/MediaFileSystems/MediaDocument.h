#pragma once
#include "Representations.h"
#include "Volumes.h"
#include "MessageRecipe.h"
#include <map>
#include <string>

namespace RetroMediaLab::MediaFileSystems {
class SourceDescriptor {
public:
    virtual ~SourceDescriptor() = default;
    virtual const std::string& primaryPath() const = 0;
    virtual std::optional<std::int64_t> knownLength() const = 0;
};
class ImageDocument {
public:
    virtual ~ImageDocument() = default;
    virtual const SourceDescriptor& source() const = 0;
    virtual const std::string& formatId() const = 0;
    virtual bool isTape() const = 0;
    virtual bool isHardDisk() const = 0;
    virtual const ImageRepresentation& representation() const = 0;
    virtual std::span<const VolumeDescriptor> volumes() const = 0;
    virtual std::span<const std::string> diagnostics() const = 0;
    virtual std::span<const MessageRecipe> messages() const { return {}; }
    virtual const std::map<std::string, std::string>& metadata() const = 0;
};
}
