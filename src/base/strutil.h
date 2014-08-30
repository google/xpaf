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

#ifndef XPAF_BASE_STRUTIL_H_
#define XPAF_BASE_STRUTIL_H_

#include <sstream>
#include <string>

#include "base/integral_types.h"
#include "base/stl_decl.h"
#include "base/stringpiece.h"

namespace xpaf {

inline string SimpleItoa(int i) {
  stringstream ss;
  ss << i;
  return ss.str();
}

// Copies the string pointed to by src (including terminating NUL char) to the
// array pointed to by dest. If strlen(s) > n-1, only the first n-1 bytes are
// copied and the last byte is set to NUL. Returns dest.
//
// Differs from strncpy() in the following ways:
//  * The remainder of dest will not be padded with NUL chars.
//  * If n >= 1, dest will always be NUL-terminated.
inline char* safestrncpy(char* dest, const char* src, size_t n) {
  if (n < 1) return dest;

  // Avoid using non-ANSI memccpy(), which is also deprecated in MSVC.
  for (size_t i = 0; i < n; ++i) {
    if ((dest[i] = src[i]) == '\0')
      return dest;
  }

  dest[n-1] = '\0';
  return dest;
}

// A buffer size large enough for all FastToBuffer functions.
const int kFastToBufferSize = 32;

// Writes output to the beginning of the given buffer. Returns a pointer to the
// end of the string (i.e. to the NUL char). Buffer must be at least 12 bytes.
// Not actually fast, but maybe someday!
inline char* FastInt32ToBuffer(int32 i, char* buffer) {
  const string s = SimpleItoa(i);
  safestrncpy(buffer, s.c_str(), s.size());
  return buffer + s.size();
}

inline bool HasPrefixString(const StringPiece& str,
                            const StringPiece& prefix) {
  return str.starts_with(prefix);
}

inline bool HasSuffixString(const StringPiece& str,
                            const StringPiece& suffix) {
  return str.ends_with(suffix);
}

// Functions with AlphaNum params can accept numerical values; these are
// automatically converted to strings.
// Designed for internal use by StrCat().
struct AlphaNum {
 public:
  StringPiece piece;
  char digits[kFastToBufferSize];

  AlphaNum(int32 i)
      : piece(digits, FastInt32ToBuffer(i, digits) - &digits[0]) {}
  AlphaNum(const char* c_str) : piece(c_str) {}
  AlphaNum(const StringPiece& pc) : piece(pc) {}
  AlphaNum(const string& s) : piece(s) {}

  StringPiece::size_type size() const { return piece.size(); }
  const char* data() const { return piece.data(); }

 private:
  AlphaNum(char c);  // use ":", not ':'
};

// Merges the given strings or numbers, with no delimiter.
//
string StrCat(const AlphaNum& a, const AlphaNum& b);
string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c);
string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d);
string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e);
string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e, const AlphaNum& f);
string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e, const AlphaNum& f,
              const AlphaNum& g);

}  // namespace xpaf

#endif  // XPAF_BASE_STRUTIL_H_
