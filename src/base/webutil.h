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

#ifndef XPAF_BASE_WEBUTIL_H_
#define XPAF_BASE_WEBUTIL_H_

#include "base/integral_types.h"

namespace xpaf {

class HTTPUtils {
 public:
  // Takes a pointer to the full HTTP document text (including headers) and
  // returns a pointer to the body of the text (possibly an empty string).
  // Returns NULL if the end of headers was not found. In this case, depending
  // on the application, the caller might check whether the initial characters
  // resemble an HTTP status line, and if not, assume the HTTP headers are
  // missing and that all of content is body.
  //
  // Note: The implementation is based on a simple scan for "\r\n\r\n", and
  // makes no attempt to parse the headers or detect a malformed HTTP response.
  //
  static const char* SkipHttpHeaders(const char* content, uint32 content_len);
};

}  // namespace xpaf

#endif  // XPAF_BASE_WEBUTIL_H_
