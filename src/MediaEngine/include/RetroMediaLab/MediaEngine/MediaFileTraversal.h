#pragma once
#include "Export.h"
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
namespace RetroMediaLab::MediaEngine {
namespace FileTraversalDetails {
struct ChildrenView {const void* data;std::size_t size;const void* (*at)(const void*,std::size_t);};
class Cursor final {
public:
 RETROMEDIALAB_MEDIAENGINE_API Cursor(const void*,std::function<bool(const void*)>,std::function<ChildrenView(const void*)>);
 RETROMEDIALAB_MEDIAENGINE_API ~Cursor();
 RETROMEDIALAB_MEDIAENGINE_API const void* next();
 Cursor(const Cursor&)=delete;Cursor& operator=(const Cursor&)=delete;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
}
// Single-pass iterators borrow entries and contiguous child collections; callbacks are owned.
template<class Entry,class Predicate,class Children> class MediaFileRange final {
 using ChildRange=std::invoke_result_t<Children&,const Entry&>;
 static_assert(std::ranges::contiguous_range<ChildRange> && std::ranges::borrowed_range<ChildRange>);
 static_assert(std::is_same_v<std::remove_cv_t<std::ranges::range_value_t<ChildRange>>,Entry>);
 struct Callbacks {Predicate predicate;Children children;};
 const Entry* root_;std::shared_ptr<Callbacks> callbacks_;
public:
 class Iterator {
  std::shared_ptr<FileTraversalDetails::Cursor> cursor_;const Entry* current_=nullptr;
 public:
  using iterator_category=std::input_iterator_tag;using iterator_concept=std::input_iterator_tag;using value_type=Entry;using difference_type=std::ptrdiff_t;
  Iterator()=default;
  explicit Iterator(std::shared_ptr<FileTraversalDetails::Cursor> cursor):cursor_(std::move(cursor)),current_(static_cast<const Entry*>(cursor_->next())){}
  const Entry& operator*()const{return *current_;}const Entry* operator->()const{return current_;}
  Iterator& operator++(){current_=static_cast<const Entry*>(cursor_->next());return *this;}void operator++(int){++*this;}
  friend bool operator==(const Iterator& iterator,std::default_sentinel_t){return iterator.current_==nullptr;}
 };
 MediaFileRange(const Entry& root,Predicate predicate,Children children):root_(&root),callbacks_(std::make_shared<Callbacks>(Callbacks{std::move(predicate),std::move(children)})){}
 Iterator begin()const{
  auto callbacks=callbacks_;return Iterator(std::make_shared<FileTraversalDetails::Cursor>(root_,[callbacks](const void* value){return std::invoke(callbacks->predicate,*static_cast<const Entry*>(value));},[callbacks](const void* value){auto&& children=std::invoke(callbacks->children,*static_cast<const Entry*>(value));return FileTraversalDetails::ChildrenView{std::ranges::data(children),std::ranges::size(children),[](const void* data,std::size_t index)->const void*{return static_cast<const Entry*>(data)+index;}};}));
 }
 std::default_sentinel_t end()const noexcept{return {};}
};
template<class Entry,class Predicate,class Children> auto enumerateMediaFiles(Entry& root,Predicate predicate,Children children){return MediaFileRange<std::remove_cv_t<Entry>,Predicate,Children>(root,std::move(predicate),std::move(children));}
}
