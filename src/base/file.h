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

#ifndef XPAF_BASE_FILE_H_
#define XPAF_BASE_FILE_H_

#include <string>
#include <vector>

#include "base/stl_decl.h"

namespace xpaf {

class File {
 public:
  // Initializes the file subsystem.
  static void Init();

  // Returns the suffix for "fname", i.e. everything after the final "/".
  static string Basename(const string& fname);

  // Returns true if the given file exists, false otherwise.
  static bool Exists(const string& fname);

  // Reads the content of file "fname" into "output". Clobbers any existing data
  // in "output". Returns true on success, false otherwise.
  static bool ReadFileToString(const string& fname, string* output);

  // Calls CHECK(ReadFileToString(fname, output)).
  static void ReadFileToStringOrDie(const string& fname, string* output);

  // Appends the names of all files matching "pattern" to "output".
  // Returns true on success, false on failure. Finding no files is not
  // considered a failure.
  // Note: This function skips dot files and backup files.
  static bool Match(const string& pattern, vector<string>* output);
};

}  // namespace xpaf

#endif  // XPAF_BASE_FILE_H_
