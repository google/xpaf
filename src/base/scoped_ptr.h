// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Similar to C++0x unique_ptr class.

#ifndef XPAF_BASE_SCOPED_PTR_H_
#define XPAF_BASE_SCOPED_PTR_H_

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

namespace xpaf {

// A scoped_ptr<T> is like a T*, except that the destructor of scoped_ptr<T>
// automatically deletes the pointer it holds (if any).
// That is, scoped_ptr<T> owns the T object that it points to.
// Like T*, scoped_ptr<T> may hold either NULL or a pointer to a T object.
// Also like T*, scoped_ptr<T> is thread-compatible, and once you dereference
// it, you get the thread-safety guarantees of T.
//
// Size: sizeof(scoped_ptr<C>) == sizeof(C*)
template <class C>
class scoped_ptr {
 public:
  // Constructor. Defaults to initializing with NULL.
  // There is no way to create an uninitialized scoped_ptr. The input parameter
  // must be allocated with new.
  explicit scoped_ptr(C* p = NULL) : ptr_(p) { }

  // Destructor. Deletes the owned object, if any.
  // We don't test ptr_ == NULL because C++ does that for us.
  ~scoped_ptr() {
    enum { type_must_be_complete = sizeof(C) };
    delete ptr_;
  }

  // Reset. Deletes the owned object, if any. Then, takes ownership of a new
  // object, if given. this->reset(this->get()) works.
  void reset(C* p = NULL) {
    if (p != ptr_) {
      enum { type_must_be_complete = sizeof(C) };
      delete ptr_;
      ptr_ = p;
    }
  }

  // Accessors to get the owned object.
  // operator* and operator-> will assert() if there is no owned object.
  C& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }
  C* operator->() const  {
    assert(ptr_ != NULL);
    return ptr_;
  }
  C* get() const { return ptr_; }

  // Comparison operators.
  // These return whether a scoped_ptr and a raw pointer refer to the same
  // object, not just to two different but equal objects.
  bool operator==(const C* p) const { return ptr_ == p; }
  bool operator!=(const C* p) const { return ptr_ != p; }

  // Swaps two scoped pointers.
  void swap(scoped_ptr& p2) {
    C* tmp = ptr_;
    ptr_ = p2.ptr_;
    p2.ptr_ = tmp;
  }

  // Releases a pointer.
  // The return value is the current pointer held by this object. If this object
  // holds a NULL pointer, the return value is NULL. After this operation, this
  // object will hold a NULL pointer, and will not own any objects.
  C* release() {
    C* retVal = ptr_;
    ptr_ = NULL;
    return retVal;
  }

 private:
  C* ptr_;

  // Forbid comparison of scoped_ptr types. If C2 != C, it totally doesn't make
  // sense, and if C2 == C, it still doesn't make sense since you should never
  // have the same object owned by two different scoped_ptrs.
  template <class C2> bool operator==(scoped_ptr<C2> const& p2) const;
  template <class C2> bool operator!=(scoped_ptr<C2> const& p2) const;

  // Disallow copy and assign.
  scoped_ptr(const scoped_ptr&);
  void operator=(const scoped_ptr&);
};

// Free functions.
template <class C>
inline void swap(scoped_ptr<C>& p1, scoped_ptr<C>& p2) {
  p1.swap(p2);
}

template <class C>
inline bool operator==(const C* p1, const scoped_ptr<C>& p2) {
  return p1 == p2.get();
}

template <class C>
inline bool operator==(const C* p1, const scoped_ptr<const C>& p2) {
  return p1 == p2.get();
}

template <class C>
inline bool operator!=(const C* p1, const scoped_ptr<C>& p2) {
  return p1 != p2.get();
}

template <class C>
inline bool operator!=(const C* p1, const scoped_ptr<const C>& p2) {
  return p1 != p2.get();
}

}  // namespace xpaf

#endif  // XPAF_BASE_SCOPED_PTR_H_
