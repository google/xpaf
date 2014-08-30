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

#ifndef XPAF_UTIL_H_
#define XPAF_UTIL_H_

#include <string>
#include <vector>

#include "base/stl_decl.h"

namespace xpaf {

class Document;
class XpafParserDefs;

// Reads the given file with format "[url]\n[http_response_with_headers]" and
// constructs a new Document for it. Caller takes ownership of returned
// Document. Input strings 'url' and 'content' must persist for the lifetime of
// the returned Document.
Document* MakeDocFromFile(const string& file_path,
                          string* url,
                          string* content);

// Reads all files matching 'file_glob', parses each one into an XpafParserDefs
// proto, and merges them all into *parser_defs. Input *parser_defs is not
// cleared prior to merging.
void ReadXpafParserDefs(const string& file_glob, XpafParserDefs* parser_defs);

}  // namespace xpaf

#endif  // XPAF_UTIL_H_
