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

// Example:
// ./parse_tool
//      --input_file_path=./testing/testdata/twitter_bradfitz.http
//      --parser_defs_glob=./testing/parser_defs/*.xpd

#include <stdio.h>

#include <string>

#include "base/commandlineflags.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_decl.h"
#include "base/strutil.h"
#include "document.h"
#include "parsed_document.pb.h"
#include "util.h"
#include "xpaf_parser.h"
#include "xpaf_parser_def.pb.h"
#include "xpaf_parser_master.h"

DEFINE_string(input_file_path, "",
              "Path of file to parse.");
DEFINE_string(parser_defs_glob, "",
              "File pattern for parser def files. E.g., '/path/to/*.xpd'.");
DEFINE_bool(abort_on_parse_error, false,
            "If true, we abort on parse errors.");

namespace xpaf {

void Run() {
  File::Init();

  CHECK(!FLAGS_input_file_path.empty());
  CHECK(!FLAGS_parser_defs_glob.empty());

  XpafParserDefs parser_defs;
  ReadXpafParserDefs(FLAGS_parser_defs_glob, &parser_defs);

  ParseOptions opt;
  if (FLAGS_abort_on_parse_error) {
    opt.error_handling_mode = EHM_ABORT_PROCESS;
  }
  XpafParserMaster master(parser_defs, opt);

  string url, content;
  scoped_ptr<Document> doc(
      MakeDocFromFile(FLAGS_input_file_path, &url, &content));

  ParsedDocument parsed_document;
  master.ParseDocument(*doc, &parsed_document);
  printf("%s", parsed_document.DebugString().c_str());
}

}  // namespace xpaf

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  xpaf::Run();
  return 0;
}
