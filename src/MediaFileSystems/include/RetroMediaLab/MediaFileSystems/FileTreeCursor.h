#pragma once
#include "Export.h"
#include <cstddef>
#include <functional>
#include <memory>
namespace RetroMediaLab::MediaFileSystems {
struct FileChildrenView {const void* data=nullptr;std::size_t size=0;const void* (*at)(const void*,std::size_t)=nullptr;};
using FilePredicate=std::function<bool(const void*)>;
using FileChildren=std::function<FileChildrenView(const void*)>;
// Entries and child storage remain borrowed until iteration ends.
class FileTreeCursor final {
public:
 RETROMEDIALAB_MEDIAFILESYSTEMS_API FileTreeCursor(const void* root,FilePredicate,FileChildren);
 RETROMEDIALAB_MEDIAFILESYSTEMS_API ~FileTreeCursor();
 RETROMEDIALAB_MEDIAFILESYSTEMS_API const void* next();
 FileTreeCursor(const FileTreeCursor&)=delete;
 FileTreeCursor& operator=(const FileTreeCursor&)=delete;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
}
