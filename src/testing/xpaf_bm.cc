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

// To run benchmarks:
// ./xpaf_bm

#include <string>
#include <vector>

#include "base/benchmark.h"
#include "base/commandlineflags.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_decl.h"
#include "base/stl_util.h"
#include "base/strutil.h"
#include "document.h"
#include "parsed_document.pb.h"
#include "util.h"
#include "xpaf_parser.h"
#include "xpaf_parser_def.pb.h"
#include "xpaf_parser_master.h"
#include "xpath_wrapper.h"

DEFINE_string(file_name, "",
              "Name of http file to parse (excluding extension). If empty, we"
              " parse all http files.");

// TODO(sadovsky): This should probably be set based on SRCDIR env var.
// http://www.gnu.org/s/hello/manual/automake/Simple-Tests.html
DEFINE_string(test_srcdir, "./", "");

namespace xpaf {
namespace {

const char* kDataDir = "/src/testing/xpaf_bm_data";

void BM_NewXPathWrapper(int iters) {
  StopBenchmarkTiming();
  File::Init();
  const string data_dir = FLAGS_test_srcdir + kDataDir;

  vector<string> http_files;
  if (!FLAGS_file_name.empty()) {
    http_files.push_back(StrCat(data_dir, "/", FLAGS_file_name, ".http"));
  } else {
    File::Match(data_dir + "/*.http", &http_files);
  }
  CHECK(!http_files.empty()) << "No http files found!";

  StartBenchmarkTiming();
  for (int i = 0; i < http_files.size(); ++i) {
    string url, content;
    scoped_ptr<Document> doc(MakeDocFromFile(http_files[i], &url, &content));
    for (int j = 0; j < iters; ++j) {
      scoped_ptr<XPathWrapper> wrapper(
          XPathWrapper::NewXPathWrapper(url, content, doc->content_type()));
    }
  }
}
BENCHMARK(BM_NewXPathWrapper);

void BM_XpafParserMasterCtor(int iters) {
  StopBenchmarkTiming();
  File::Init();
  const string data_dir = FLAGS_test_srcdir + kDataDir;

  XpafParserDefs parser_defs;
  ReadXpafParserDefs(data_dir + "/*.xpd", &parser_defs);
  CHECK_GT(parser_defs.parser_defs_size(), 0) << "No parser defs found!";

  StartBenchmarkTiming();
  vector<string> names;
  for (int i = 0; i < iters; ++i) {
    const XpafParserMaster master(parser_defs, ParseOptions());
    master.ParserNames(&names);
  }
}
BENCHMARK(BM_XpafParserMasterCtor);

void BM_XpafParserMasterParse(int iters) {
  StopBenchmarkTiming();
  File::Init();
  const string data_dir = FLAGS_test_srcdir + kDataDir;

  XpafParserDefs parser_defs;
  ReadXpafParserDefs(data_dir + "/*.xpd", &parser_defs);
  CHECK_GT(parser_defs.parser_defs_size(), 0) << "No parser defs found!";

  vector<string> http_files;
  if (!FLAGS_file_name.empty()) {
    http_files.push_back(StrCat(data_dir, "/", FLAGS_file_name, ".http"));
  } else {
    File::Match(data_dir + "/*.http", &http_files);
  }
  CHECK(!http_files.empty()) << "No http files found!";

  vector<string*> url_vec;
  vector<string*> content_vec;
  vector<Document*> docs;
  for (int i = 0; i < http_files.size(); ++i) {
    url_vec.push_back(new string());
    content_vec.push_back(new string());
    docs.push_back(MakeDocFromFile(http_files[i],
                                   url_vec.back(),
                                   content_vec.back()));
  }

  const XpafParserMaster master(parser_defs, ParseOptions());

  StartBenchmarkTiming();
  for (int i = 0; i < iters; ++i) {
    for (int j = 0; j < docs.size(); ++j) {
      ParsedDocument parsed_doc;
      master.ParseDocument(*docs[j], &parsed_doc);
    }
  }

  STLDeleteElements(&docs);
  STLDeleteElements(&content_vec);
  STLDeleteElements(&url_vec);
}
BENCHMARK(BM_XpafParserMasterParse);

}  // namespace
}  // namespace xpaf
