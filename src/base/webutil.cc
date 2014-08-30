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

#include "base/webutil.h"

#include <string.h>  // for memchr

#include "base/integral_types.h"

namespace xpaf {
namespace {

// Finds "\r\n\r\n" in str.
const char* find_rnrn(const char* str, size_t n) {
  const char* where = NULL;

  if (n < 4)
    return NULL;

  n -= 3;
  while ((where = static_cast<const char*>(memchr(str, '\r', n))) != NULL) {
    if (where[1] == '\n' &&
        where[2] == '\r' &&
        where[3] == '\n') {
      return const_cast<char*>(where);
    }
    n -= (where + 1) - str;
    str = where + 1;
  }

  return NULL;
}

// Finds "\n\n" in str.
const char* find_nn(const char* str, size_t n) {
  const char* where = NULL;

  if (n < 2)
    return NULL;

  n -= 1;
  while ((where = static_cast<const char*>(memchr(str, '\n', n))) != NULL) {
    if (where[1] == '\n') {
      return where;
    }
    n -= (where + 1) - str;
    str = where + 1;
  }

  return NULL;
}

}  // namespace


/* static */
const char* HTTPUtils::SkipHttpHeaders(const char* content,
                                       uint32 content_len) {
  const char* found;
  found = find_rnrn(content, content_len);
  if (found == NULL) {
    found = find_nn(content, content_len);
    if (found == NULL) {
      return NULL;
    }
    return found + 2;
  }
  return found + 4;
}

}  // namespace xpaf
