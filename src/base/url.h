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

// Dummy implementation of URL class for use in open source release.
// API is a tiny subset of Google's URL class API.
//
// NOTE: This implementation does not correctly resolve all relative urls.
// We may want to consider using http://code.google.com/p/google-url/.

#ifndef XPAF_BASE_URL_H_
#define XPAF_BASE_URL_H_

#include <string>

#include "base/macros.h"
#include "base/stl_decl.h"
#include "base/stringpiece.h"
#include "base/strutil.h"

namespace xpaf {

class URL {
 public:
  // Constructs a URL from the url string "url".
  URL(const StringPiece& url)
      : url_(url.data(), url.size()) {}

  // Constructs a URL from "url", using "base_url" to turn relative components
  // into absolute components.
  // TODO(sadovsky): This implementation is simplistic and often incorrect; it
  // should be replaced.
  // See http://tools.ietf.org/html/rfc3986#section-5.2 for proper algorithm.
  URL(const URL& base_url, const StringPiece& url) {
    const size_t url_slash_idx = url.find('/');
    if (url_slash_idx != StringPiece::npos && url_slash_idx != 0) {
      url_ = url.as_string();
      return;
    }
    const size_t last_slash_idx = base_url.url_.rfind('/');
    if (last_slash_idx == string::npos) {
      url_ = url.as_string();
      return;
    }
    const size_t substr_len = last_slash_idx + (url_slash_idx == 0 ? 0 : 1);
    url_ = StrCat(base_url.url_.substr(0, substr_len), url);
  }

  const char* Assemble() { return url_.c_str(); }

  string AssembleString() const { return url_; }

  bool is_valid() const { return true; }

 private:
  // For our dummy implementation, internal representation is just a string.
  string url_;

  DISALLOW_COPY_AND_ASSIGN(URL);
};

}  // namespace xpaf

#endif  // XPAF_BASE_URL_H_
