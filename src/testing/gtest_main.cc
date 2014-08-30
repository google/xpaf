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

#include <gtest/gtest.h>

#include "base/commandlineflags.h"
#include "base/logging.h"

// TODO(sadovsky): This should probably be set based on SRCDIR env var.
// http://www.gnu.org/s/hello/manual/automake/Simple-Tests.html
DEFINE_string(test_srcdir, "./", "");

int main(int argc, char** argv) {
  // Run death tests in a way that's guaranteed to be thread-safe.
  // Without this, gtest warns that it can't detect the number of threads.
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  ::testing::InitGoogleTest(&argc, argv);
  google::ParseCommandLineFlags(&argc, &argv, true);
  return RUN_ALL_TESTS();
}
