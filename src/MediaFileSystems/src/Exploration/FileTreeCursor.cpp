#include <RetroMediaLab/MediaFileSystems/FileTreeCursor.h>
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
struct FileTreeCursor::Impl {
 struct Frame {const void* entry;bool tested=false,loaded=false;FileChildrenView children;std::size_t position=0;};
 FilePredicate isFile;FileChildren children;std::vector<Frame> frames;
};
FileTreeCursor::FileTreeCursor(const void* root,FilePredicate predicate,FileChildren children):impl_(std::make_unique<Impl>()){
 if(!root||!predicate||!children)throw MediaError(DiagnosticCode::InvalidSource);impl_->isFile=std::move(predicate);impl_->children=std::move(children);impl_->frames.push_back({root});
}
FileTreeCursor::~FileTreeCursor()=default;
const void* FileTreeCursor::next(){
 try{while(!impl_->frames.empty()){
  auto& frame=impl_->frames.back();if(!frame.tested){frame.tested=true;if(impl_->isFile(frame.entry))return frame.entry;}
  if(!frame.loaded){frame.children=impl_->children(frame.entry);frame.loaded=true;if(frame.children.size&&(!frame.children.data||!frame.children.at))throw MediaError(DiagnosticCode::InvalidSource);}
  if(frame.position==frame.children.size){impl_->frames.pop_back();continue;}
  const auto child=frame.children.at(frame.children.data,frame.position++);if(!child)throw MediaError(DiagnosticCode::InvalidSource);impl_->frames.push_back({child});
 }return nullptr;}catch(...){impl_->frames.clear();throw;}
}
}
