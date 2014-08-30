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

// Tests for XpafParser::Init(). Currently, we only test that Init() dies when
// it encounters an invalid XpafParserDef.

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <re2/re2.h>

#include "base/commandlineflags.h"
#include "base/file.h"
#include "base/strutil.h"
#include "util.h"
#include "xpaf_parser.h"
#include "xpaf_parser_def.pb.h"

DECLARE_string(test_srcdir);

namespace xpaf {
namespace {

const char* kDataDir = "/src/testing/init_test_data";

TEST(InitTest, All) {
  File::Init();

  XpafParserDefs parser_defs;
  ReadXpafParserDefs(FLAGS_test_srcdir + kDataDir + "/*.xpd", &parser_defs);
  EXPECT_GT(parser_defs.parser_defs_size(), 0) << "No parser defs found!";

  for (int i = 0; i < parser_defs.parser_defs_size(); ++i) {
    const XpafParserDef& parser_def = parser_defs.parser_defs(i);
    XpafParser parser;
    if (!parser_def.has_userdata()) {
      // No userdata means Init() should succeed.
      parser.Init(parser_def, ParseOptions());
    } else {
      // Some XpafParser error messages contain the XpafParserDef DebugString,
      // so we explicitly disallow quotes around the expected error regexp to
      // avoid matching the DebugString.
      const string expected_error_regexp =
          StrCat("[^\"]", parser_def.userdata(), "[^\"]");
      EXPECT_DEATH(parser.Init(parser_def, ParseOptions()),
                   expected_error_regexp) << parser_def.DebugString();
    }
  }
}

// Tests that the DebugString of a proto string field contains the field value
// with quotes around it. The test above assumes this property.
TEST(InitTest, ProtoDebugStringHasQuotes) {
  XpafParserDef def;
  def.set_userdata("foo");
  const string debug_string = def.DebugString();
  EXPECT_FALSE(RE2::PartialMatch(debug_string, "[^\"]foo[^\"]"));
  EXPECT_TRUE(RE2::PartialMatch(debug_string, "\"foo\""));
}

}  // namespace
}  // namespace xpaf
