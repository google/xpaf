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

// To run benchmarks: ./re2_strstr_bm

#include <re2/re2.h>

#include "base/benchmark.h"
#include "base/logging.h"
#include "base/strutil.h"

namespace xpaf {
namespace {

void BM_RE2_PartialMatch(int iters) {
  char url[] = "http://www.quora.com/foo/bar/baz/boo-faz-zip";
  const RE2 regexp("quora\\.com/");
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    url[11] = 'a' + (i & 15);
    if (RE2::PartialMatch(url, regexp)) {
      ++count;
    }
  }
  CHECK_GT(count, -1);
}
BENCHMARK(BM_RE2_PartialMatch);

void BM_RE2_PartialMatch_Prefix(int iters) {
  char url[] = "http://www.quora.com/foo/bar/baz/boo-faz-zip";
  const RE2 regexp("^http://(:?[^/]+\\.)?quora\\.[^/]+/");
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    url[11] = 'a' + (i & 15);
    if (RE2::PartialMatch(url, regexp)) {
      ++count;
    }
  }
  CHECK_GT(count, -1);
}
BENCHMARK(BM_RE2_PartialMatch_Prefix);

void BM_RE2_FullMatch(int iters) {
  char url[] = "http://www.quora.com/foo/bar/baz/boo-faz-zip";
  const RE2 regexp(".*quora.com/.*");
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    url[11] = 'a' + (i & 15);
    if (RE2::FullMatch(url, regexp)) {
      ++count;
    }
  }
  CHECK_GT(count, -1);
}
BENCHMARK(BM_RE2_FullMatch);

void BM_Strstr(int iters) {
  char url[] = "http://www.quora.com/foo/bar/baz/boo-faz-zip";
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    url[11] = 'a' + (i & 15);
    if (strstr(url, "quora.com/") != NULL) {
      ++count;
    }
  }
  CHECK_GT(count, -1);
}
BENCHMARK(BM_Strstr);

}  // namespace
}  // namespace xpaf
