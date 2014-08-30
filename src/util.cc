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

#include "util.h"

#include <string>
#include <vector>

#include <google/protobuf/text_format.h>

#include "base/file.h"
#include "base/logging.h"
#include "base/stl_decl.h"
#include "document.h"
#include "xpaf_parser_def.pb.h"

namespace xpaf {

Document* MakeDocFromFile(const string& file_path,
                          string* url,
                          string* content) {
  string tmp;
  File::ReadFileToStringOrDie(file_path, &tmp);
  const size_t endl_pos = tmp.find('\n');
  *url = tmp.substr(0, endl_pos);
  *content = tmp.substr(endl_pos + 1);

  CHECK(!url->empty());
  CHECK(!content->empty());

  // NOTE(sadovsky): Currently we assume doc is UTF-8 encoded.
  Document* doc = new Document();
  doc->Init(*url, *content, CONTENT_TYPE_HTML);
  return doc;
}

void ReadXpafParserDefs(const string& file_glob, XpafParserDefs* parser_defs) {
  vector<string> file_paths;
  CHECK(File::Match(file_glob, &file_paths)) << file_glob;
  for (int i = 0; i < file_paths.size(); ++i) {
    string parser_defs_str;
    File::ReadFileToStringOrDie(file_paths[i], &parser_defs_str);
    CHECK(google::protobuf::TextFormat::MergeFromString(parser_defs_str,
                                                        parser_defs))
        << file_paths[i] << "\n" << parser_defs_str;
  }
}

}  // namespace xpaf
