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

#ifndef XPAF_XPAF_PARSER_H_
#define XPAF_XPAF_PARSER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/stl_decl.h"
#include "xpaf_parser_def.pb.h"

namespace xpaf {

class ParserOutput;
class QueryInfo;
class QueryResultsCache;
class StringPiece;
class XPathWrapper;

namespace internal { class QueryRunner; }

typedef unordered_map<string, const QueryInfo*> QueryInfoMap;
typedef vector<pair<string, bool> > QueryResults;

enum ErrorHandlingMode {
  EHM_IGNORE = 0,     // silently skip record
  EHM_LOG_ERROR,      // log error and skip record
  EHM_ABORT_PROCESS,  // log error and abort process
};

struct ParseOptions {
  ErrorHandlingMode error_handling_mode;

  ParseOptions()
      : error_handling_mode(EHM_LOG_ERROR) {}
};

// Thread-safe after Init() has returned and before destructor has been called.
class XpafParser {
 public:
  XpafParser();
  ~XpafParser();

  // Initializes this parser. Must be called before any other method, and must
  // only be called once.
  void Init(const XpafParserDef& parser_def,
            const ParseOptions& parse_options);

  // Returns the name of this parser.
  string ParserName() const;

  // Returns true if Parse() should be called for the given document, based on
  // our XpafParserDef's url_regexp.
  // This function is kept separate from Parse() so that users can avoid
  // constructing XPathWrappers for documents that won't be parsed.
  bool ShouldParse(const StringPiece& url) const;

  // Parses the given document, populating 'output'.
  // Does not check ShouldParse().
  void Parse(const StringPiece& url,
             const XPathWrapper& xpath_wrapper,
             ParserOutput* output) const;

 private:
  // Init() helper.
  void ProcessReference(string* ref,
                        int* num_inlined_queries,
                        unordered_map<string, string>* inlined_query_refs);

  // Parse() helper.
  const QueryResults* GetQueryResults(const StringPiece& url,
                                      const internal::QueryRunner& query_runner,
                                      const string& key,
                                      QueryResultsCache* cache) const;

  // True if Init() has been called. Not lock-protected because it's only
  // modified by Init(), and we only guarantee thread-safety after Init() has
  // returned.
  bool initialized_;

  // Definition proto for this parser, set by Init().
  XpafParserDef parser_def_;

  // Parsing options, set by Init().
  ParseOptions parse_options_;

  // QueryDefs created by Init() for inlined queries.
  vector<QueryDef*> inlined_query_defs_;

  // Maps query key to const QueryInfo*. Populated by Init().
  QueryInfoMap query_info_map_;

  DISALLOW_COPY_AND_ASSIGN(XpafParser);
};

}  // namespace xpaf

#endif  // XPAF_XPAF_PARSER_H_
