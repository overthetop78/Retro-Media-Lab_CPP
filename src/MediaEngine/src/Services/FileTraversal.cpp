#include <RetroMediaLab/MediaEngine/MediaFileTraversal.h>
#include <RetroMediaLab/MediaFileSystems/FileTreeCursor.h>
namespace RetroMediaLab::MediaEngine::FileTraversalDetails {
struct Cursor::Impl {MediaFileSystems::FileTreeCursor cursor;Impl(const void* root,std::function<bool(const void*)> predicate,std::function<ChildrenView(const void*)> children):cursor(root,std::move(predicate),[children=std::move(children)](const void* value){const auto view=children(value);return MediaFileSystems::FileChildrenView{view.data,view.size,view.at};}){}};
Cursor::Cursor(const void* root,std::function<bool(const void*)> predicate,std::function<ChildrenView(const void*)> children):impl_(std::make_unique<Impl>(root,std::move(predicate),std::move(children))){}
Cursor::~Cursor()=default;
const void* Cursor::next(){return impl_->cursor.next();}
}
