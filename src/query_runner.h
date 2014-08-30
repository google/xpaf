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

#ifndef XPAF_QUERY_RUNNER_H_
#define XPAF_QUERY_RUNNER_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/scoped_ptr.h"
#include "xpaf_parser.h"  // for ErrorHandlingMode

namespace xpaf {

class QueryDef;
class QueryGroupDef;
class StringPiece;
class URL;
class XPathWrapper;

namespace internal {

typedef vector<pair<string, bool> > QueryResults;

class QueryRunner {
 public:
  // Note: 'url' and 'xpath_wrapper' must persist for the lifetime of this
  // object.
  QueryRunner(const StringPiece& url,
              const XPathWrapper& xpath_wrapper,
              ErrorHandlingMode error_handling_mode);

  ~QueryRunner();

  void RunStandaloneQuery(const QueryDef& query_def,
                          QueryResults* results) const;

  void RunGroupedQueries(const QueryGroupDef& query_group_def,
                         vector<QueryResults*>* results_vec) const;

 private:
  // Takes const char* rather than const string& to avoid an extra conversion.
  bool PostProcessResult(const QueryDef& query_def,
                         const char* orig_result,
                         string* processed_result) const;

  // Used by RunStandaloneQuery() but not RunGroupedQueries().
  void PostProcessAndAppendResult(const QueryDef& query_def,
                                  const char* result_str,
                                  QueryResults* results) const;

  void RunGroupedQueriesLogError(const QueryGroupDef& query_group_def,
                                 const StringPiece& error) const;

  const StringPiece& url_;
  const scoped_ptr<const URL> url_obj_;

  const XPathWrapper& xpath_wrapper_;
  const ErrorHandlingMode error_handling_mode_;

  DISALLOW_COPY_AND_ASSIGN(QueryRunner);
};

}  // namespace internal
}  // namespace xpaf

#endif  // XPAF_QUERY_RUNNER_H_
