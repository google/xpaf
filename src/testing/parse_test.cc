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

#include <string>
#include <vector>

#include <google/protobuf/text_format.h>
#include <gtest/gtest.h>
#include <re2/re2.h>

#include "base/commandlineflags.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/strutil.h"
#include "document.h"
#include "parsed_document.pb.h"
#include "util.h"
#include "xpaf_parser.h"
#include "xpaf_parser_def.pb.h"
#include "xpaf_parser_master.h"

DECLARE_string(test_srcdir);
DEFINE_string(test_name, "",
              "Name of test to run. We'll parse [test_name].http using all"
              " known parsers and compare the output against [test_name].out."
              " If empty, we run all tests.");
DEFINE_bool(log_parse_errors, false,
            "If true, OutputCorrectness test will use EHM_LOG_ERROR instead of"
            " EHM_IGNORE.");

namespace xpaf {
namespace {

const char* kDataDir = "/src/testing/parse_test_data";

bool ParserOutputPrecedes(const ParserOutput* a, const ParserOutput* b) {
  return a->parser_name() < b->parser_name();
}

class ParseTest : public ::testing::Test {
 public:
  ParseTest() : data_dir_(FLAGS_test_srcdir + kDataDir) {
    File::Init();

    ReadXpafParserDefs(data_dir_ + "/*.xpd", &parser_defs_);
    EXPECT_GT(parser_defs_.parser_defs_size(), 0) << "No parser defs found!";

    if (!FLAGS_test_name.empty()) {
      http_files_.push_back(StrCat(data_dir_, "/", FLAGS_test_name, ".http"));
    } else {
      File::Match(data_dir_ + "/*.http", &http_files_);
    }
    EXPECT_FALSE(http_files_.empty()) << "No test cases found!";
  }

 protected:
  const string data_dir_;
  XpafParserDefs parser_defs_;
  vector<string> http_files_;
};

// XpafParser assumes a certain order for ErrorHandlingMode enum values.
TEST(ErrorHandlingMode, ValueOrder) {
  EXPECT_GT(EHM_ABORT_PROCESS, EHM_LOG_ERROR);
  EXPECT_GT(EHM_LOG_ERROR, EHM_IGNORE);
}

// Parses each http file with all parsers at once and checks that the output
// matches the corresponding out file.
TEST_F(ParseTest, OutputCorrectness) {
  if (!FLAGS_test_name.empty()) {
    // Check that the number of out files matches the number of http files. For
    // each http file we'll read the corresponding out file, so this just
    // ensures that there are no out files without a corresponding http file.
    vector<string> out_files;
    File::Match(data_dir_ + "/*.out", &out_files);
    EXPECT_EQ(http_files_.size(), out_files.size())
        << "There's an out file with no corresponding http file!";
  }

  ParseOptions opt;
  opt.error_handling_mode = (FLAGS_log_parse_errors ?
                             EHM_LOG_ERROR : EHM_IGNORE);
  const XpafParserMaster master(parser_defs_, opt);

  for (int i = 0; i < http_files_.size(); ++i) {
    string test_name = File::Basename(http_files_[i]);
    test_name = test_name.substr(0, test_name.size() - 5);  // strip ".http"
    const string out_file = StrCat(data_dir_, "/", test_name, ".out");
    LOG(INFO) << "Testing " << test_name;

    string url, content;
    scoped_ptr<Document> doc(MakeDocFromFile(http_files_[i], &url, &content));

    string out_str;
    File::ReadFileToStringOrDie(out_file, &out_str);
    ParsedDocument expected;
    CHECK(google::protobuf::TextFormat::ParseFromString(out_str, &expected))
        << out_file << "\n" << out_str;

    ParsedDocument actual;
    master.ParseDocument(*doc, &actual);

    // Sort parser outputs within actual and expected, since XpafParserMaster
    // shuffles them.
    // NOTE(sadovsky): It would be simpler to use pointer_begin() and
    // pointer_end(), but these aren't available in libprotobuf-dev 2.2.0.
    sort(expected.mutable_parser_outputs()->mutable_data(),
         expected.mutable_parser_outputs()->mutable_data()
         + expected.parser_outputs_size(),
         &ParserOutputPrecedes);
    sort(actual.mutable_parser_outputs()->mutable_data(),
         actual.mutable_parser_outputs()->mutable_data()
         + actual.parser_outputs_size(),
         &ParserOutputPrecedes);

    EXPECT_EQ(actual.DebugString(), expected.DebugString());
  }
}

// Helper function for BrokenParsersAbort test.
void ParseHttpFiles(const XpafParserMaster& master,
                    const vector<string>& http_files) {
  for (int i = 0; i < http_files.size(); ++i) {
    string url, content;
    scoped_ptr<Document> doc(MakeDocFromFile(http_files[i], &url, &content));
    ParsedDocument result;
    master.ParseDocument(*doc, &result);
  }
}

// For each XpafParserDef, creates an XpafParserMaster just for that def, and
// then checks that the parser aborts iff it claims that it should.
TEST_F(ParseTest, BrokenParsersAbort) {
  for (int i = 0; i < parser_defs_.parser_defs_size(); ++i) {
    XpafParserDefs one_def;
    one_def.add_parser_defs()->CopyFrom(parser_defs_.parser_defs(i));

    ParseOptions opt;
    opt.error_handling_mode = EHM_ABORT_PROCESS;
    XpafParserMaster master(one_def, opt);

    if (parser_defs_.parser_defs(i).userdata() == "should_abort") {
      EXPECT_DEATH(ParseHttpFiles(master, http_files_), "");
    } else {
      // Shouldn't die.
      ParseHttpFiles(master, http_files_);
    }
  }
}

}  // namespace
}  // namespace xpaf
