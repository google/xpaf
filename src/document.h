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

// Defines a Document class that represents a single interchange-valid UTF-8
// encoded document.

#ifndef XPAF_DOCUMENT_H_
#define XPAF_DOCUMENT_H_

#include "base/macros.h"
#include "base/stringpiece.h"

namespace xpaf {

// Note: Enum values are subject to change.
enum ContentType {
  CONTENT_TYPE_UNKNOWN = 0,
  CONTENT_TYPE_HTML,
  CONTENT_TYPE_XML,
};

class Document {
 public:
  Document() {}
  ~Document() {}

  // Note: Input url and content must persist for the lifetime of this object.
  // They are not copied.
  void Init(const StringPiece& url,
            const StringPiece& content,
            ContentType content_type) {
    url_ = url;
    content_ = content;
    content_type_ = content_type;
  }

  StringPiece url() const {
    return url_;
  }

  StringPiece content() const {
    return content_;
  }

  ContentType content_type() const {
    return content_type_;
  }

 private:
  StringPiece url_;
  StringPiece content_;
  ContentType content_type_;

  DISALLOW_COPY_AND_ASSIGN(Document);
};

}  // namespace xpaf

#endif  // XPAF_DOCUMENT_H_
