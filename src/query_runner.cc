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

#include "query_runner.h"

#include <string>
#include <utility>
#include <vector>

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <re2/re2.h>

// TODO(sadovsky): Require "xpaf/" prefix for xpaf includes.

#include "base/callback.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_decl.h"
#include "base/stringpiece.h"
#include "base/strutil.h"
#include "base/url.h"
#include "post_processing_ops.pb.h"
#include "xpaf_parser_def.pb.h"
#include "xpaf_parser.h"  // for ErrorHandlingMode
#include "xpath_wrapper.h"

namespace xpaf {
namespace internal {

namespace {

const char* kInvalidPostProcessingOpError =
    "Invalid post-processing op. XpafParser::Init() should've caught this.";

bool QueryReturnsUrls(const string& query) {
  return HasSuffixString(query, "/@href") || HasSuffixString(query, "/@src");
}

// Converts 'url' into an absolute url with 'base_url' as its base.
// Return value indicates whether conversion succeeded. If this returns false,
// *absolute_url should not be used.
bool AbsolutizeUrl(const string& url,
                   const URL& base_url,
                   string* absolute_url) {
  URL absolute_url_obj(base_url, url);
  *absolute_url = absolute_url_obj.Assemble();
  return absolute_url_obj.is_valid();
}

}  // namespace

QueryRunner::QueryRunner(const StringPiece& url,
                         const XPathWrapper& xpath_wrapper,
                         ErrorHandlingMode error_handling_mode)
    : url_(url),
      url_obj_(new URL(url_)),
      xpath_wrapper_(xpath_wrapper),
      error_handling_mode_(error_handling_mode) {
}

QueryRunner::~QueryRunner() {
}

// Processes 'orig_result' and populates 'processed_result'.
// Return value indicates whether processing succeeded. If this returns false,
// *processed_result will be unchanged.
bool QueryRunner::PostProcessResult(const QueryDef& query_def,
                                    const char* orig_result,
                                    string* processed_result) const {
  bool ok = true;
  string in;
  string out = orig_result;
  if (QueryReturnsUrls(query_def.query())) {
    in.swap(out);
    ok = AbsolutizeUrl(in, *url_obj_, &out);
  }
  for (int i = 0; i < query_def.post_processing_ops_size(); ++i) {
    if (!ok) break;
    const PostProcessingOp& op = query_def.post_processing_ops(i);
    // TODO(sadovsky): Implement SubstrOp and ConvertOp.
    if (op.has_replace_op()) {
      if (op.replace_op().global()) {
        RE2::GlobalReplace(&out, op.replace_op().regexp(),
                           op.replace_op().rewrite());
      } else {
        RE2::Replace(&out, op.replace_op().regexp(),
                     op.replace_op().rewrite());
      }
    } else if (op.has_extract_op()) {
      in.swap(out);
      ok = RE2::PartialMatch(in, op.extract_op().regexp(), &out);
    } else if (op.has_substr_op()) {
      LOG(FATAL) << kInvalidPostProcessingOpError;
    } else if (op.has_convert_op()) {
      LOG(FATAL) << kInvalidPostProcessingOpError;
    } else {
      LOG(FATAL) << kInvalidPostProcessingOpError;
    }
  }

  if (!ok) {
    if (error_handling_mode_ >= EHM_LOG_ERROR) {
      LOG(ERROR) << "PostProcessResult() failed"
                 << "\nurl: " << url_
                 << "\norig_result: " << orig_result
                 << "\nquery_def:\n" << query_def.DebugString();
    }
    if (error_handling_mode_ == EHM_ABORT_PROCESS) LOG(FATAL);
  } else {
    processed_result->swap(out);
  }

  VLOG(1) << query_def.name() << ": " << (ok ? *processed_result : "NULL");
  return ok;
}

inline void QueryRunner::PostProcessAndAppendResult(
    const QueryDef& query_def,
    const char* result_str,
    QueryResults* results) const {
  string processed_result;
  const bool ok = PostProcessResult(query_def, result_str, &processed_result);
  results->push_back(make_pair(ok ? processed_result : "", ok));
}

void QueryRunner::RunStandaloneQuery(const QueryDef& query_def,
                                     QueryResults* results) const {
  DCHECK(results->empty());

  xmlXPathObjectPtr xpath_obj =
      xpath_wrapper_.EvalExpressionOrDie(query_def.query());
  const AutoClosureRunner xpath_obj_deleter(
      NewCallback(&xmlXPathFreeObject, xpath_obj));
  if (xpath_obj->type == XPATH_BOOLEAN) {
    // Note: We could use xmlXPathCastBooleanToString, but that returns "true"
    // or "false". "1" or "0" is more concise.
    PostProcessAndAppendResult(
        query_def, xpath_obj->boolval ? "1" : "0", results);
  } else if (xpath_obj->type == XPATH_NUMBER) {
    xmlChar* str = xmlXPathCastNumberToString(xpath_obj->floatval);
    PostProcessAndAppendResult(
        query_def, reinterpret_cast<const char*>(str), results);
    xmlFree(str);
  } else if (xpath_obj->type == XPATH_STRING) {
    PostProcessAndAppendResult(
        query_def, reinterpret_cast<const char*>(xpath_obj->stringval),
        results);
  } else if (xpath_obj->type == XPATH_NODESET) {
    if (xpath_obj->nodesetval != NULL) {
      for (int i = 0; i < xpath_obj->nodesetval->nodeNr; ++i) {
        xmlNodePtr node = xpath_obj->nodesetval->nodeTab[i];
        xmlChar* content = xmlNodeGetContent(node);
        if (content != NULL) {
          PostProcessAndAppendResult(query_def,
                                     reinterpret_cast<const char*>(content),
                                     results);
        }
        xmlFree(content);
      }
    }
  } else {
    LOG(FATAL) << "Can't handle xmlXPathObjectType: " << xpath_obj->type;
  }

  VLOG(1) << "Got " << results->size() << " results for query: "
          << query_def.query();
}

void QueryRunner::RunGroupedQueriesLogError(
    const QueryGroupDef& query_group_def, const StringPiece& error) const {
  if (error_handling_mode_ >= EHM_LOG_ERROR) {
    LOG(ERROR) << "Error in RunGroupedQueries()"
               << "\nurl: " << url_
               << "\nerror: " << error
               << "\nquery_group_def: " << query_group_def.DebugString();
  }
  if (error_handling_mode_ == EHM_ABORT_PROCESS) LOG(FATAL);
}

void QueryRunner::RunGroupedQueries(const QueryGroupDef& query_group_def,
                                    vector<QueryResults*>* results_vec) const {
  const int num_subqueries = query_group_def.query_defs_size();
  DCHECK_EQ(num_subqueries, results_vec->size());
  DCHECK_GE(num_subqueries, 1);

  const string& root_query = query_group_def.root_query();
  xmlXPathObjectPtr xpath_obj = xpath_wrapper_.EvalExpressionOrDie(root_query);
  const AutoClosureRunner xpath_obj_deleter(
      NewCallback(&xmlXPathFreeObject, xpath_obj));
  if (xpath_obj->type != XPATH_NODESET) {
    RunGroupedQueriesLogError(query_group_def, "Root query must return nodes");
    // Effect of error: All subqueries will have no results.
    return;
  } else if (xpath_obj->nodesetval == NULL) {
    // Root query has no results.
    return;
  }

  const int num_results_per_subquery = xpath_obj->nodesetval->nodeNr;
  VLOG(1) << "Got " << num_results_per_subquery << " results for root query: "
          << root_query;

  // Map each xmlNodePtr to its result index.
  unordered_map<xmlNodePtr, int> root_node_to_index;
  for (int i = 0; i < num_results_per_subquery; ++i) {
    CHECK(root_node_to_index.insert(
        make_pair(xpath_obj->nodesetval->nodeTab[i], i)).second);
  }

  // Run each subquery.
  for (int i = 0; i < num_subqueries; ++i) {
    const string subquery = StrCat(root_query,
                                   query_group_def.query_defs(i).query());
    QueryResults* results = (*results_vec)[i];
    DCHECK(results != NULL && results->empty());

    // Initialize all results to ("", false).
    results->assign(num_results_per_subquery, make_pair("", false));

    xmlXPathObjectPtr subquery_xpath_obj =
        xpath_wrapper_.EvalExpressionOrDie(subquery);
    const AutoClosureRunner subquery_xpath_obj_deleter(
        NewCallback(&xmlXPathFreeObject, subquery_xpath_obj));
    if (subquery_xpath_obj->type != XPATH_NODESET) {
      // NOTE(sadovsky): I'm not sure this can actually happen, conditional on
      // subquery being a valid XPath expression.
      RunGroupedQueriesLogError(
          query_group_def,
          StrCat("Subquery ", subquery, " must return nodes"));
      // Effect of error: Current subquery will have no results.
      continue;
    } else if (subquery_xpath_obj->nodesetval == NULL) {
      // Subquery has no results.
      continue;
    }

    const int num_subquery_results = subquery_xpath_obj->nodesetval->nodeNr;
    VLOG(1) << "Got " << num_subquery_results << " results for subquery: "
            << subquery;

    if (num_subquery_results > num_results_per_subquery &&
        error_handling_mode_ >= EHM_LOG_ERROR) {
      // Log a warning, but let the consequences play out below. By the
      // pigeonhole principle, at least one result has no root node or has the
      // same root node as some other result, so we'll definitely trigger an
      // error below.
      LOG(WARNING) << StrCat("Subquery ", subquery, " has more results than"
                             " root query (", num_subquery_results, " > ",
                             num_results_per_subquery, ")");
    }

    // For each subquery result, find the corresponding root node, get its
    // index, and save the result to this index.
    vector<bool> used_root_node_indices(num_results_per_subquery, false);
    for (int j = 0; j < num_subquery_results; ++j) {
      const xmlNodePtr orig_node = subquery_xpath_obj->nodesetval->nodeTab[j];
      xmlNodePtr curr_node = orig_node;
      unordered_map<xmlNodePtr, int>::const_iterator it =
          root_node_to_index.end();
      while (curr_node != NULL) {
        it = root_node_to_index.find(curr_node);
        if (it != root_node_to_index.end()) break;
        curr_node = curr_node->parent;
      }
      if (curr_node == NULL) {
        // This can happen, for example, if root_query is "//span" and subquery
        // is "/parent::*".
        RunGroupedQueriesLogError(
            query_group_def,
            StrCat("Failed to find root node for subquery ", subquery,
                   " result index ", j));
        // Effect of error: Current subquery result will be missing.
        continue;
      }
      const int root_node_index = it->second;
      if (used_root_node_indices[root_node_index]) {
        RunGroupedQueriesLogError(
            query_group_def,
            StrCat("Result index ", j, " for subquery ", subquery, " has same"
                   " root node (index ", root_node_index, ") as some other"
                   " result"));
        // Effect of error: Current result index will retain old value.
        continue;
      }
      used_root_node_indices[root_node_index] = true;
      xmlChar* content = xmlNodeGetContent(orig_node);
      if (content != NULL) {
        pair<string, bool>* result = &(*results)[root_node_index];
        DCHECK(!result->second);
        string processed_result;
        const bool ok =
            PostProcessResult(query_group_def.query_defs(i),
                              reinterpret_cast<const char*>(content),
                              &processed_result);
        if (ok) {
          result->first.swap(processed_result);
          result->second = true;
        }
      }
      xmlFree(content);
    }
  }
}

}  // namespace internal
}  // namespace xpaf
