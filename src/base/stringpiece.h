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

// A string-like object that points to a sized piece of memory.
//
// Functions or methods may use a const StringPiece& parameter to accept either
// a "const char*" or a "string" value that will be implicitly converted to a
// StringPiece. The implicit conversion means that it is often appropriate to
// include this .h file in other files rather than forward-declaring StringPiece
// as would be appropriate for most other classes.
//
// Systematic usage of StringPiece is encouraged as it will reduce unnecessary
// conversions from "const char*" to "string" and back again.

#ifndef XPAF_BASE_STRINGPIECE_H_
#define XPAF_BASE_STRINGPIECE_H_

#include <stddef.h>
#include <string.h>

#include <algorithm>
#include <iosfwd>
#include <string>

namespace xpaf {

class StringPiece {
 private:
  const char* ptr_;
  int length_;

 public:
  // We provide non-explicit singleton constructors so that users can pass in a
  // "const char*" or a "string" wherever a "StringPiece" is expected.
  StringPiece()
      : ptr_(NULL), length_(0) {}
  StringPiece(const char* str)
      : ptr_(str),
        length_((str == NULL) ? 0 : static_cast<int>(strlen(str))) {}
  StringPiece(const std::string& str)
      : ptr_(str.data()),
        length_(static_cast<int>(str.size())) {}
  StringPiece(const char* offset, int len)
      : ptr_(offset), length_(len) {}

  // Note: data() may return a pointer to a buffer with embedded NULs, and the
  // returned buffer may or may not be NUL-terminated. Therefore, it is
  // typically a mistake to pass data() to a routine that expects a NUL-
  // terminated string.
  const char* data() const { return ptr_; }
  int size() const { return length_; }
  int length() const { return length_; }
  bool empty() const { return length_ == 0; }

  void clear() { ptr_ = NULL; length_ = 0; }
  void set(const char* data, int len) { ptr_ = data; length_ = len; }
  void set(const char* str) {
    ptr_ = str;
    if (str != NULL)
      length_ = static_cast<int>(strlen(str));
    else
      length_ = 0;
  }
  void set(const void* data, int len) {
    ptr_ = reinterpret_cast<const char*>(data);
    length_ = len;
  }

  char operator[](int i) const { return ptr_[i]; }

  void remove_prefix(int n) {
    ptr_ += n;
    length_ -= n;
  }

  void remove_suffix(int n) {
    length_ -= n;
  }

  int compare(const StringPiece& x) const {
    int r = memcmp(ptr_, x.ptr_, std::min(length_, x.length_));
    if (r == 0) {
      if (length_ < x.length_) r = -1;
      else if (length_ > x.length_) r = +1;
    }
    return r;
  }

  // Note: as_string() and ToString() are identical.
  std::string as_string() const {
    return std::string(data(), size());
  }
  std::string ToString() const {
    return std::string(data(), size());
  }

  void CopyToString(std::string* target) const;
  void AppendToString(std::string* target) const;

  // Does "this" start with "x"?
  bool starts_with(const StringPiece& x) const {
    return ((length_ >= x.length_) &&
            (memcmp(ptr_, x.ptr_, x.length_) == 0));
  }

  // Does "this" end with "x"?
  bool ends_with(const StringPiece& x) const {
    return ((length_ >= x.length_) &&
            (memcmp(ptr_ + (length_ - x.length_), x.ptr_, x.length_) == 0));
  }

  // Standard STL container boilerplate.
  typedef char value_type;
  typedef const char* pointer;
  typedef const char& reference;
  typedef const char& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  static const size_type npos;
  typedef const char* const_iterator;
  typedef const char* iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  iterator begin() const { return ptr_; }
  iterator end() const { return ptr_ + length_; }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(ptr_ + length_);
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(ptr_);
  }
  // STL says return size_type, but Google says return int.
  int max_size() const { return length_; }
  int capacity() const { return length_; }

  int copy(char* buf, size_type n, size_type pos = 0) const;

  int find(const StringPiece& s, size_type pos = 0) const;
  int find(char c, size_type pos = 0) const;
  int rfind(const StringPiece& s, size_type pos = npos) const;
  int rfind(char c, size_type pos = npos) const;

  StringPiece substr(size_type pos, size_type n = npos) const;

  static bool _equal(const StringPiece&, const StringPiece&);
};

inline bool operator==(const StringPiece& x, const StringPiece& y) {
  return StringPiece::_equal(x, y);
}

inline bool operator!=(const StringPiece& x, const StringPiece& y) {
  return !(x == y);
}

inline bool operator<(const StringPiece& x, const StringPiece& y) {
  const int r = memcmp(x.data(), y.data(),
                       std::min(x.size(), y.size()));
  return ((r < 0) || ((r == 0) && (x.size() < y.size())));
}

inline bool operator>(const StringPiece& x, const StringPiece& y) {
  return y < x;
}

inline bool operator<=(const StringPiece& x, const StringPiece& y) {
  return !(x > y);
}

inline bool operator>=(const StringPiece& x, const StringPiece& y) {
  return !(x < y);
}

}  // namespace xpaf

// Allow StringPiece to be logged.
extern std::ostream& operator<<(std::ostream& o,
                                const xpaf::StringPiece& piece);

#endif  // XPAF_BASE_STRINGPIECE_H_
