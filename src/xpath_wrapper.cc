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

#include "xpath_wrapper.h"

#include <string>

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "base/logging.h"
#include "base/stl_decl.h"
#include "base/stringpiece.h"
#include "base/webutil.h"
#include "document.h"

namespace xpaf {

XPathWrapper::XPathWrapper(xmlDocPtr doc)
    : doc_(doc),
      context_(xmlXPathNewContext(doc)) {
}

XPathWrapper::~XPathWrapper() {
  xmlXPathFreeContext(context_);
  xmlFreeDoc(doc_);
}

xmlXPathObjectPtr XPathWrapper::EvalExpressionOrDie(const string& expr) const {
  VLOG(1) << "Evaluating expression: " << expr;
  xmlXPathObjectPtr xpath_obj =
      xmlXPathEvalExpression(BAD_CAST expr.c_str(), context_);
  CHECK(xpath_obj != NULL) << "Invalid expression: " << expr;
  return xpath_obj;
}

/* static */
XPathWrapper* XPathWrapper::NewXPathWrapper(const StringPiece& url,
                                            const StringPiece& content,
                                            ContentType content_type) {
  const char* content_data = content.data();
  const char* body = HTTPUtils::SkipHttpHeaders(content_data, content.size());
  uint32 body_size = content.size();
  if (body == NULL) {
    body = content_data;
  } else {
    body_size -= (body - content_data);
  }

  if (content_type == CONTENT_TYPE_HTML) {
    htmlDocPtr doc_ptr = htmlReadMemory(
        body, body_size, url.data(), NULL /* encoding */,
        HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
    return new XPathWrapper(doc_ptr);
  } else if (content_type == CONTENT_TYPE_XML) {
    xmlDocPtr doc_ptr = xmlReadMemory(
        body, body_size, url.data(), NULL /* encoding */,
        XML_PARSE_NONET);
    return new XPathWrapper(doc_ptr);
  } else {
    return NULL;
  }
}

}  // namespace xpaf
