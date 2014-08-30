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

#include "base/strutil.h"

#include <string>

#include "base/logging.h"
#include "base/stl_util.h"

namespace xpaf {

static char* Append1(char* out, const AlphaNum& x) {
  memcpy(out, x.data(), x.size());
  return out + x.size();
}

static char* Append2(char* out, const AlphaNum& x1, const AlphaNum& x2) {
  memcpy(out, x1.data(), x1.size());
  out += x1.size();

  memcpy(out, x2.data(), x2.size());
  return out + x2.size();
}

static char *Append4(char* out,
                     const AlphaNum& x1, const AlphaNum& x2,
                     const AlphaNum& x3, const AlphaNum& x4) {
  memcpy(out, x1.data(), x1.size());
  out += x1.size();

  memcpy(out, x2.data(), x2.size());
  out += x2.size();

  memcpy(out, x3.data(), x3.size());
  out += x3.size();

  memcpy(out, x4.data(), x4.size());
  return out + x4.size();
}

string StrCat(const AlphaNum& a, const AlphaNum& b) {
  string result;
  STLStringResizeUninitialized(&result, a.size() + b.size());
  char* const begin = &*result.begin();
  char* out = Append2(begin, a, b);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c) {
  string result;
  STLStringResizeUninitialized(&result, a.size() + b.size() + c.size());
  char* const begin = &*result.begin();
  char* out = Append2(begin, a, b);
  out = Append1(out, c);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d) {
  string result;
  STLStringResizeUninitialized(
      &result, a.size() + b.size() + c.size() + d.size());
  char* const begin = &*result.begin();
  char* out = Append4(begin, a, b, c, d);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e) {
  string result;
  STLStringResizeUninitialized(
      &result, a.size() + b.size() + c.size() + d.size() + e.size());
  char* const begin = &*result.begin();
  char* out = Append4(begin, a, b, c, d);
  out = Append1(out, e);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e, const AlphaNum& f) {
  string result;
  STLStringResizeUninitialized(
      &result, a.size() + b.size() + c.size() + d.size() + e.size() + f.size());
  char* const begin = &*result.begin();
  char* out = Append4(begin, a, b, c, d);
  out = Append2(out, e, f);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

string StrCat(const AlphaNum& a, const AlphaNum& b, const AlphaNum& c,
              const AlphaNum& d, const AlphaNum& e, const AlphaNum& f,
              const AlphaNum& g) {
  string result;
  STLStringResizeUninitialized(
      &result, (a.size() + b.size() + c.size() + d.size() + e.size() +
                f.size() + g.size()));
  char* const begin = &*result.begin();
  char* out = Append4(begin, a, b, c, d);
  out = Append2(out, e, f);
  out = Append1(out, g);
  DCHECK_EQ(out, begin + result.size());
  return result;
}

}  // namespace xpaf
