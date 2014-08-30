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

#ifndef XPAF_BASE_STL_UTIL_H_
#define XPAF_BASE_STL_UTIL_H_

#include <string>

namespace xpaf {

inline void STLStringResizeUninitialized(string* s, size_t new_size) {
  s->resize(new_size);
}

// For a range within a container of pointers, calls delete (non-array version)
// on these pointers.
// NOTE: We delete behind the iterator, since deleting a value can invalidate
// the iterator (e.g. if the key is a pointer into the value object).
template <class ForwardIterator>
void STLDeleteContainerPointers(ForwardIterator begin,
                                ForwardIterator end) {
  while (begin != end) {
    ForwardIterator temp = begin;
    ++begin;
    delete *temp;
  }
}

// For a range within a container of pairs, calls delete (non-array version) on
// the SECOND item in the pairs.
// NOTE: Like STLDeleteContainerPointers(), we delete behind the iterator.
template <class ForwardIterator>
void STLDeleteContainerPairSecondPointers(ForwardIterator begin,
                                          ForwardIterator end) {
  while (begin != end) {
    ForwardIterator temp = begin;
    ++begin;
    delete temp->second;
  }
}

// Deletes all the elements in an STL container and clears the container. This
// function is suitable for use with a vector, set, unordered_set, or any other
// STL container which defines sensible begin(), end(), and clear() methods.
// If container is NULL, this function does nothing.
template <class T>
void STLDeleteElements(T *container) {
  if (!container) return;
  STLDeleteContainerPointers(container->begin(), container->end());
  container->clear();
}

// Given an STL container consisting of (key, value) pairs, deletes all the
// "value" components and clears the container.
// If container is NULL, this function does nothing.
template <class T>
void STLDeleteValues(T *v) {
  if (!v) return;
  STLDeleteContainerPairSecondPointers(v->begin(), v->end());
  v->clear();
}

}  // namespace xpaf

#endif  // XPAF_BASE_STL_UTIL_H_
