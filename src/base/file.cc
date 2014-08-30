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

#include "base/file.h"

#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <string>
#include <vector>

#include "base/logging.h"

namespace xpaf {

/* static */
void File::Init() {
}

/* static */
string File::Basename(const string& fname) {
  const size_t last = fname.rfind('/');
  if (last == string::npos)
    return fname;
  else if (last == fname.size() - 1)
    return string();
  return fname.substr(last + 1);
}

/* static */
bool File::Exists(const string& fname) {
  struct stat buf;
  return stat(fname.c_str(), &buf) == 0;
}

/* static */
bool File::ReadFileToString(const string& fname, string* output) {
  const int kBufsize = 1024;
  char buffer[kBufsize];
  FILE* file = fopen(fname.c_str(), "rb");
  if (file == NULL) return false;
  while (true) {
    const size_t n = fread(buffer, 1, kBufsize, file);
    if (n <= 0) break;
    output->append(buffer, n);
  }
  const int error = ferror(file);
  if (fclose(file) != 0) return false;
  return error == 0;
}

/* static */
void File::ReadFileToStringOrDie(const string& fname, string* output) {
  CHECK(ReadFileToString(fname, output)) << "Failed to read file: " << fname;
}

namespace {

// Used by Match(). We use this function rather than just passing GLOB_ERR and
// checking the return code because with GLOB_ERR, glob() returns GLOB_ABORTED
// for "no-such-directory/*"; we want it to return GLOB_NOMATCH.
int GlobErrFunc(const char* path, int eerrno) {
  // ENOENT is "no such file or directory".
  // EACCES is "permission denied".
  return (eerrno == ENOENT || eerrno == EACCES) ? 0 : 1;
}

void FilterMatches(const string& pattern, vector<string>* output, int start) {
  int dst = start;
  for (int i = start; i < output->size(); ++i) {
    const string& fname = (*output)[i];
    if (File::Basename(fname.c_str())[0] == '.') {
      continue;
    }
    if (!fname.empty() && (fname[fname.size() - 1] == '~')) {
      continue;
    }
    if (i != dst) {
      (*output)[dst].swap((*output)[i]);
    }
    ++dst;
  }
  output->resize(dst);
}

}  // namespace

/* static */
bool File::Match(const string& pattern, vector<string>* output) {
  if (output == NULL) return false;
  glob_t g;
  const int orig_size = output->size();
  const int result = glob(pattern.c_str(), 0, GlobErrFunc, &g);
  if (result == 0) {
    for (int i = 0; i < g.gl_pathc; i++) {
      // Insert only if the path is not empty, since glob() returns "" as a
      // matched path if pattern is an empty string.
      if (strlen(g.gl_pathv[i]) != 0)
        output->push_back(string(g.gl_pathv[i]));
    }
    FilterMatches(pattern, output, orig_size);
  }
  globfree(&g);
  return result == 0 || result == GLOB_NOMATCH;
}

}  // namespace xpaf
