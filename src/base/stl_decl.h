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

#ifndef XPAF_BASE_STL_DECL_H_
#define XPAF_BASE_STL_DECL_H_

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef __clang__
#include <unordered_map>
#include <unordered_set>
#else
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif  // __clang__

using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::ostream;
using std::pair;
using std::set;
using std::sort;
using std::string;
using std::stringstream;
using std::swap;
using std::vector;

#ifdef __clang__
using std::unordered_map;
using std::unordered_set;
#else
using std::tr1::unordered_map;
using std::tr1::unordered_set;
#endif  // __clang__

#endif  // XPAF_BASE_STL_DECL_H_
