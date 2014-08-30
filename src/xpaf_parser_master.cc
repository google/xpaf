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

#include "xpaf_parser_master.h"

#include <string>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_decl.h"
#include "base/stl_util.h"
#include "base/stringpiece.h"
#include "document.h"
#include "parsed_document.pb.h"
#include "xpaf_parser.h"
#include "xpaf_parser_def.pb.h"
#include "xpath_wrapper.h"

namespace xpaf {

XpafParserMaster::XpafParserMaster(const XpafParserDefs& parser_defs,
                                   const ParseOptions& parse_options) {
  CHECK_GT(parser_defs.parser_defs_size(), 0);
  for (int i = 0; i < parser_defs.parser_defs_size(); ++i) {
    XpafParser* parser = new XpafParser();
    parser->Init(parser_defs.parser_defs(i), parse_options);
    CHECK(parser_map_.insert(make_pair(parser->ParserName(), parser)).second)
        << "Duplicate parser name " << parser->ParserName();
  }
}

XpafParserMaster::~XpafParserMaster() {
  STLDeleteValues(&parser_map_);
}

bool XpafParserMaster::ShouldParse(const StringPiece& url) const {
  for (ParserMap::const_iterator it = parser_map_.begin();
       it != parser_map_.end(); ++it) {
    if (it->second->ShouldParse(url)) {
      return true;
    }
  }
  return false;
}

void XpafParserMaster::ParseDocument(const Document& doc,
                                     ParsedDocument* parsed_document) const {
  parsed_document->set_url(doc.url().as_string());

  if (doc.content_type() != CONTENT_TYPE_HTML &&
      doc.content_type() != CONTENT_TYPE_XML) {
    return;
  }

  // Find all parsers that should parse this document.
  vector<const XpafParser*> relevant_parsers;
  for (ParserMap::const_iterator it = parser_map_.begin();
       it != parser_map_.end(); ++it) {
    if (it->second->ShouldParse(doc.url())) {
      VLOG(2) << "Relevant parser: " << it->first;
      relevant_parsers.push_back(it->second);
    }
  }

  if (relevant_parsers.empty()) {
    return;
  }

  scoped_ptr<XPathWrapper> xpath_wrapper(XPathWrapper::NewXPathWrapper(
      doc.url(), doc.content(), doc.content_type()));

  for (vector<const XpafParser*>::const_iterator it = relevant_parsers.begin();
       it != relevant_parsers.end(); ++it) {
    const XpafParser* parser = *it;
    ParserOutput* output = parsed_document->add_parser_outputs();
    parser->Parse(doc.url(), *xpath_wrapper, output);
    // If the ParserOutput is empty, remove it.
    if (output->relations_size() == 0) {
      parsed_document->mutable_parser_outputs()->RemoveLast();
    }
    output->set_parser_name(parser->ParserName());
  }
}

void XpafParserMaster::ParserNames(vector<string>* names) const {
  names->clear();
  names->reserve(parser_map_.size());
  for (ParserMap::const_iterator it = parser_map_.begin();
       it != parser_map_.end(); ++it) {
    names->push_back(it->second->ParserName());
  }
}

}  // namespace xpaf
