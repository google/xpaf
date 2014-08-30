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

// Modeled after re2: http://code.google.com/p/re2/

#ifndef XPAF_BASE_BENCHMARK_H_
#define XPAF_BASE_BENCHMARK_H_

#include "base/integral_types.h"

namespace xpaf {

namespace testing {
struct Benchmark {
  const char* name;
  void (*fn)(int);
  void (*fnr)(int, int);
  int lo;
  int hi;
  int threadlo;
  int threadhi;

  void Register();
  Benchmark(const char* name, void (*f)(int)) {
    Clear(name); fn = f; Register();
  }
  Benchmark(const char* name, void (*f)(int, int), int l, int h) {
    Clear(name); fnr = f; lo = l; hi = h; Register();
  }
  void Clear(const char* n) {
    name = n; fn = 0; fnr = 0; lo = 0; hi = 0; threadlo = 0; threadhi = 0;
  }
  Benchmark* ThreadRange(int lo, int hi) {
    threadlo = lo; threadhi = hi; return this;
  }
};
}  // namespace testing

void StopBenchmarkTiming();
void StartBenchmarkTiming();

void SetBenchmarkBytesProcessed(int64);
void SetBenchmarkItemsProcessed(int);

int NumCPUs();

#define BENCHMARK(f)                                                    \
  testing::Benchmark* _benchmark_##f = (new testing::Benchmark(#f, f))

#define BENCHMARK_RANGE(f, lo, hi)                                      \
  testing::Benchmark* _benchmark_##f = (new testing::Benchmark(#f, f, lo, hi))

}  // namespace xpaf

#endif  // XPAF_BASE_BENCHMARK_H_
