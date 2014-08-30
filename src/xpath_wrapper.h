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

#ifndef XPAF_XPATH_WRAPPER_H_
#define XPAF_XPATH_WRAPPER_H_

#include <string>
#include <vector>

#include <libxml/tree.h>   // for xmlDocPtr
#include <libxml/xpath.h>  // for xmlXPathContextPtr

#include "base/macros.h"
#include "base/stl_decl.h"
#include "document.h"  // for ContentType

class StringPiece;

namespace xpaf {

class XPathWrapper {
 public:
  // Takes ownership of 'doc'.
  explicit XPathWrapper(xmlDocPtr doc);

  ~XPathWrapper();

  xmlXPathObjectPtr EvalExpressionOrDie(const string& expr) const;

  // Constructs an XPathWrapper for the given document. Ignores HTTP headers.
  // Returns NULL if 'content_type' is neither HTML nor XML.
  static XPathWrapper* NewXPathWrapper(const StringPiece& url,
                                       const StringPiece& content,
                                       ContentType content_type);

 private:
  xmlDocPtr doc_;
  xmlXPathContextPtr context_;

  DISALLOW_COPY_AND_ASSIGN(XPathWrapper);
};

}  // namespace xpaf

#endif  // XPAF_XPATH_WRAPPER_H_
