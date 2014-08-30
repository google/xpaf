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

#ifndef XPAF_XPAF_PARSER_MASTER_H_
#define XPAF_XPAF_PARSER_MASTER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/stl_decl.h"

namespace xpaf {

class Document;
class ParseOptions;
class ParsedDocument;
class StringPiece;
class XpafParser;
class XpafParserDefs;

class XpafParserMaster {
 public:
  // Constructs an XpafParser for each XpafParserDef in 'parser_defs', passing
  // 'parse_options' to XpafParser::Init().
  XpafParserMaster(const XpafParserDefs& parser_defs,
                   const ParseOptions& parse_options);

  ~XpafParserMaster();

  // Returns true if any XpafParser::ShouldParse() returns true.
  bool ShouldParse(const StringPiece& url) const;

  // Parses 'doc' using all of our parsers, populating 'parsed_document'.
  void ParseDocument(const Document& doc,
                     ParsedDocument* parsed_document) const;

  // Populates 'names' with all of our parser names.
  void ParserNames(vector<string>* names) const;

 private:
  typedef unordered_map<string, const XpafParser*> ParserMap;

  ParserMap parser_map_;

  DISALLOW_COPY_AND_ASSIGN(XpafParserMaster);
};

}  // namespace xpaf

#endif  // XPAF_XPAF_PARSER_MASTER_H_
