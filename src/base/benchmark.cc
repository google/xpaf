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

#include "base/benchmark.h"

#include <stdio.h>
#include <sys/time.h>

#include <algorithm>
#include <string>

#include <re2/re2.h>

#include "base/integral_types.h"
#include "base/logging.h"
#include "base/stl_decl.h"

namespace xpaf {

using testing::Benchmark;

static Benchmark* benchmarks[10000];
static int nbenchmarks = 0;

void Benchmark::Register() {
  benchmarks[nbenchmarks] = this;
  if (lo < 1)
    lo = 1;
  if (hi < lo)
    hi = lo;
  if (threadlo < 1)
    threadlo = 1;
  if (threadhi < threadlo)
    threadhi = threadlo;
  nbenchmarks++;
}

static int64 nsec() {
  struct timeval tv;
  CHECK_NE(gettimeofday(&tv, NULL), -1);
  return static_cast<int64>(tv.tv_sec)*1000*1000*1000 + tv.tv_usec*1000;
}

static int64 ns;
static int64 t0;
static int64 bytes;
static int64 items;

void StopBenchmarkTiming() {
  if (t0 != 0)
    ns += nsec() - t0;
  t0 = 0;
}

void StartBenchmarkTiming() {
  if (t0 == 0)
    t0 = nsec();
}

void SetBenchmarkBytesProcessed(int64 x) {
  bytes = x;
}

void SetBenchmarkItemsProcessed(int n) {
  items = n;
}

int NumCPUs() {
  return 1;
}

static void runN(Benchmark* b, int n, int siz) {
  bytes = 0;
  items = 0;
  ns = 0;
  t0 = nsec();
  if (b->fn) {
    b->fn(n);
  } else if (b->fnr) {
    b->fnr(n, siz);
  } else {
    LOG(FATAL) << "Missing function: " <<  b->name;
  }
  if (t0 != 0) ns += nsec() - t0;
}

static int round(int n) {
  int base = 1;
  while (base * 10 < n)
    base *= 10;
  if (n < 2*base)
    return 2*base;
  if (n < 5*base)
    return 5*base;
  return 10*base;
}

static void RunBench(Benchmark* b, int nthread, int siz, int max_name_len) {
  int n, last;

  // TODO(sadovsky): Threaded benchmarks.
  if (nthread != 1)
    return;

  // Run once in case it's expensive.
  n = 1;
  runN(b, n, siz);
  while (ns < static_cast<int>(1e9) && n < static_cast<int>(1e9)) {
    last = n;
    if (ns/n == 0)
      n = 1e9;
    else
      n = 1e9 / (ns/n);

    n = max(last+1, min(n+n/2, 100*last));
    n = round(n);
    runN(b, n, siz);
  }

  char mb[100];
  char suf[100];
  mb[0] = '\0';
  suf[0] = '\0';
  if (ns > 0 && bytes > 0)
    snprintf(mb, sizeof mb, "\t%7.2f MB/s",
             (static_cast<double>(bytes)/1e6) / (static_cast<double>(ns)/1e9));
  if (b->fnr || b->lo != b->hi) {
    if (siz >= (1<<20))
      snprintf(suf, sizeof suf, "/%dM", siz/(1<<20));
    else if (siz >= (1<<10))
      snprintf(suf, sizeof suf, "/%dK", siz/(1<<10));
    else
      snprintf(suf, sizeof suf, "/%d", siz);
  }
  printf("%-*s\t%10lld\t%10lld ns/op%s\n",
         max_name_len + 8,
         (string(b->name) + string(suf)).c_str(),
         static_cast<int64>(n),
         static_cast<int64>(ns/n),
         mb);
  fflush(stdout);
}

static int match(const char* name, int argc, const char** argv) {
  if (argc == 1)
    return 1;
  for (int i = 1; i < argc; i++)
    if (RE2::PartialMatch(name, argv[i]))
      return 1;
  return 0;
}

}  // namespace xpaf

using namespace xpaf;  // NOLINT

int main(int argc, const char** argv) {
  int max_name_len = 0;
  for (int i = 0; i < nbenchmarks; ++i) {
    max_name_len = max(max_name_len,
                       static_cast<int>(strlen(benchmarks[i]->name)));
  }
  for (int i = 0; i < nbenchmarks; ++i) {
    Benchmark* b = benchmarks[i];
    if (match(b->name, argc, argv)) {
      for (int j = b->threadlo; j <= b->threadhi; ++j) {
        for (int k = max(b->lo, 1); k <= max(b->hi, 1); k <<= 1) {
          RunBench(b, j, k, max_name_len);
        }
      }
    }
  }
}
