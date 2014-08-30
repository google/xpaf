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

// Note: For now, we don't precompile regexps. Doing so might improve
// performance, but we should profile first.

// NOTE(sadovsky): Since RE2 defines its own StringPiece class, we must convert
// our StringPieces to RE2::StringPieces. Annoying.

#include "xpaf_parser.h"

#include <string>
#include <utility>
#include <vector>

#include <re2/re2.h>
#include <re2/stringpiece.h>

#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/stl_decl.h"
#include "base/stl_util.h"
#include "base/stringpiece.h"
#include "base/strutil.h"
#include "base/url.h"
#include "parsed_document.pb.h"
#include "post_processing_ops.pb.h"
#include "query_runner.h"
#include "xpaf_parser_def.pb.h"
#include "xpath_wrapper.h"

namespace xpaf {

using internal::QueryRunner;

namespace {

const char* kDoubleInitError = "You've already called XpafParser::Init().";
const char* kForgotInitError = "You didn't call XpafParser::Init().";
const char* kInvalidReference = "Invalid reference: ";

}  // namespace

// Stores pointer to QueryDef or QueryGroupDef associated with a single query.
// Created by Init() and persists for the lifetime of this parser.
struct QueryInfo {
  // The QueryDef* for this query, if any.
  // Must be NULL if query_group_def is not NULL.
  const QueryDef* const query_def;

  // The QueryGroupDef* for this query, if any.
  // Must be NULL if query_def is not NULL.
  const QueryGroupDef* const query_group_def;

  QueryInfo(const QueryDef* query_def, const QueryGroupDef* query_group_def)
      : query_def(query_def),
        query_group_def(query_group_def) {
  }
};

class QueryResultsCache {
 public:
  QueryResultsCache() {}

  ~QueryResultsCache() {
    STLDeleteValues(&cache_);
  }

  // Returns NULL if key is not found.
  const QueryResults* Get(const string& key) {
    unordered_map<string, const QueryResults*>::const_iterator it =
        cache_.find(key);
    if (it == cache_.end()) return NULL;
    return it->second;
  }

  bool Contains(const string& key) {
    return cache_.find(key) != cache_.end();
  }

  // Takes ownership of query_results.
  void Insert(const string& key, const QueryResults* query_results) {
    DCHECK(!Contains(key));
    cache_[key] = query_results;
  }

 private:
  unordered_map<string, const QueryResults*> cache_;
};

XpafParser::XpafParser() : initialized_(false) {
}

XpafParser::~XpafParser() {
  STLDeleteValues(&query_info_map_);
  STLDeleteElements(&inlined_query_defs_);
}


////////////////////////////////////////////////////////////////////////////////
// Init() and its helpers

namespace {

string GetGroupedQueryName(const QueryGroupDef& query_group_def,
                           const QueryDef& query_def) {
  return StrCat(query_group_def.name(), ".", query_def.name());
}

bool IsQueryRef(const string& str) {
  if (str.size() < 2) return false;
  return str[0] == '%' && str[str.size() - 1] == '%';
}

string RefFromQueryName(const string& ref) {
  return StrCat("%", ref, "%");
}

// LazyRE2 kQueryNameRE = { "[A-Za-z_]+" };
static const RE2 kQueryNameRE("[A-Za-z_]+");

void CheckQueryNameFormat(const string& name) {
  CHECK(RE2::FullMatch(name, kQueryNameRE))
      << "Invalid query name: " << name;
}

void ValidateQueryDef(const QueryDef& query_def) {
  CheckQueryNameFormat(query_def.name());
  for (int i = 0; i < query_def.post_processing_ops_size(); ++i) {
    const PostProcessingOp& op = query_def.post_processing_ops(i);
    int num_ops = 0;
    if (op.has_replace_op()) {
      ++num_ops;
    }
    if (op.has_extract_op()) {
      ++num_ops;
    }
    if (op.has_substr_op()) {
      ++num_ops;
      LOG(FATAL) << "SubstrOp not yet implemented.";
    }
    if (op.has_convert_op()) {
      ++num_ops;
      LOG(FATAL) << "ConvertOp not yet implemented.";
    }
    CHECK_EQ(num_ops, 1) << "PostProcessingOp has != 1 ops\n"
                         << query_def.DebugString();
  }
}

}  // namespace

void XpafParser::ProcessReference(
    string* ref,
    int* num_inlined_queries,
    unordered_map<string, string>* inlined_query_refs) {
  if (!ref->empty()) {
    if ((*ref)[0] == '%' || (*ref)[ref->size() - 1] == '%') {
      // It's a query reference.
      // First, do some basic format checking.
      CHECK_GT(ref->size(), 2) << kInvalidReference << *ref;
      CHECK_EQ((*ref)[0], '%') << kInvalidReference << *ref;
      CHECK_EQ((*ref)[ref->size() - 1], '%') << kInvalidReference << *ref;
      // Next, make sure the reference exists in query_info_map_.
      CHECK(query_info_map_.find(*ref) != query_info_map_.end())
          << kInvalidReference << *ref;
    } else if ((*ref)[0] == '/') {
      // It's an inlined query.
      unordered_map<string, string>::const_iterator it =
          inlined_query_refs->find(*ref);
      const bool exists = it != inlined_query_refs->end();
      if (exists) {
        // We've already seen this query; don't create a new entry for it.
        *ref = it->second;
      } else {
        // Create a new QueryDef for this query and update *ref.
        const string inlined_query_name = SimpleItoa(*num_inlined_queries);
        *num_inlined_queries += 1;
        QueryDef* query_def = new QueryDef();
        query_def->set_name(inlined_query_name);
        query_def->set_query(*ref);
        inlined_query_defs_.push_back(query_def);
        *ref = RefFromQueryName(inlined_query_name);
        CHECK(query_info_map_.insert(
            make_pair(*ref, new QueryInfo(query_def, NULL))).second);
        CHECK(inlined_query_refs->insert(
            make_pair(query_def->query(), *ref)).second);
      }
    }
  }
  // It's a literal.
  if (query_info_map_.find(*ref) == query_info_map_.end()) {
    CHECK(query_info_map_.insert(
        make_pair(*ref, new QueryInfo(NULL, NULL))).second);
  }
}

void XpafParser::Init(const XpafParserDef& parser_def,
                      const ParseOptions& parse_options) {
  CHECK(!initialized_) << kDoubleInitError;
  VLOG(1) << "XpafParser[" << parser_def.parser_name() << "]::Init()";
  parser_def_.CopyFrom(parser_def);
  parse_options_ = parse_options;

  // Add predefined queries to query_info_map_.
  CHECK(query_info_map_.empty());
  CHECK(query_info_map_.insert(make_pair(RefFromQueryName("url"),
                                         new QueryInfo(NULL, NULL))).second);

  // Make sure that all user-provided QueryDefs have valid names, and add them
  // all to query_info_map_.
  for (int i = 0; i < parser_def_.query_defs_size(); ++i) {
    const QueryDef& query_def = parser_def_.query_defs(i);
    const string& query_name = query_def.name();
    ValidateQueryDef(query_def);
    CHECK(query_info_map_.insert(
        make_pair(RefFromQueryName(query_name),
                  new QueryInfo(&query_def, NULL))).second)
        << "Duplicate query name: " << query_name << "\n"
        << parser_def_.DebugString();
  }

  // Do the same for QueryGroupDefs.
  for (int i = 0; i < parser_def_.query_group_defs_size(); ++i) {
    const QueryGroupDef& query_group_def = parser_def_.query_group_defs(i);
    CheckQueryNameFormat(query_group_def.name());
    for (int j = 0; j < query_group_def.query_defs_size(); ++j) {
      ValidateQueryDef(query_group_def.query_defs(j));
      const string& grouped_query_name =
          GetGroupedQueryName(query_group_def, query_group_def.query_defs(j));
      CHECK(query_info_map_.insert(
          make_pair(RefFromQueryName(grouped_query_name),
                    new QueryInfo(NULL, &query_group_def))).second)
          << "Duplicate query name: " << grouped_query_name << "\n"
          << parser_def_.DebugString();
    }
  }

  // Iterate through relevant fields in RelationTemplates and:
  //  * Make sure all references exist in query_info_map_.
  //  * Add all literals to query_info_map_.
  //  * Create a QueryDef for each inlined query and update the corresponding
  //    RelationTemplate field to refer to it. Each new QueryDef is named with
  //    the numerical position of its corresponding query. Since user-defined
  //    Query{,Group}Def names cannot contain numbers, there's no risk of name
  //    collisions.

  // For eliminating duplicate inlined queries.
  unordered_map<string, string> inlined_query_refs;

  int num_inlined_queries = 0;
  for (int i = 0; i < parser_def_.relation_tmpls_size(); ++i) {
    RelationTemplate* rel_tmpl = parser_def_.mutable_relation_tmpls(i);
    ProcessReference(rel_tmpl->mutable_subject(),
                     &num_inlined_queries, &inlined_query_refs);
    ProcessReference(rel_tmpl->mutable_object(),
                     &num_inlined_queries, &inlined_query_refs);
    for (int j = 0; j < rel_tmpl->annotation_tmpls_size(); ++j) {
      ProcessReference(rel_tmpl->mutable_annotation_tmpls(j)->mutable_value(),
                       &num_inlined_queries, &inlined_query_refs);
    }
  }

  initialized_ = true;
}


////////////////////////////////////////////////////////////////////////////////
// ParserName() and ShouldParse()

string XpafParser::ParserName() const {
  CHECK(initialized_) << kForgotInitError;
  return parser_def_.parser_name();
}

bool XpafParser::ShouldParse(const StringPiece& url) const {
  CHECK(initialized_) << kForgotInitError;
  VLOG(1) << "XpafParser[" << ParserName() << "]::ShouldParse(" << url << ")";
  if (parser_def_.has_url_regexp() &&
      !RE2::PartialMatch(re2::StringPiece(url.data(), url.size()),
                         parser_def_.url_regexp())) {
    VLOG(1) << "  => false";
    return false;
  }
  VLOG(1) << "  => true";
  return true;
}


////////////////////////////////////////////////////////////////////////////////
// Parse() and its helpers

namespace {

void MaybeLogCardinalityOneError(const StringPiece& url,
                                 const RelationTemplate& rel_tmpl,
                                 const string& name,
                                 int results_size,
                                 ErrorHandlingMode error_handling_mode) {
  if (results_size > 1) {
    if (error_handling_mode >= EHM_LOG_ERROR) {
      LOG(ERROR) << "Too many results for " << name
                 << " (" << results_size << ")"
                 << "\nurl: " << url
                 << "\nrel_tmpl:\n" << rel_tmpl.DebugString();
    }
    if (error_handling_mode == EHM_ABORT_PROCESS) LOG(FATAL);
  }
}

void LogCardinalityManyError(const StringPiece& url,
                             const RelationTemplate& rel_tmpl,
                             const string& name,
                             int results_size,
                             int num_relations,
                             ErrorHandlingMode error_handling_mode) {
  if (error_handling_mode >= EHM_LOG_ERROR) {
    LOG(ERROR) << name << " has cardinality MANY and " << results_size
               << " results, but something else has cardinality MANY and "
               << num_relations << " results"
               << "\nurl: " << url
               << "\nrel_tmpl:\n" << rel_tmpl.DebugString();
  }
  if (error_handling_mode == EHM_ABORT_PROCESS) LOG(FATAL);
}

// Determines the number of relations to emit for the given RelationTemplate
// given the result counts and cardinalities of subject, object, and annotation
// values.
//
// The number of relations to emit is equal to the number of results in the
// first cardinality-MANY query in {subject, object, annot_1, annot_2, ...},
// with the following exceptions:
//  * If either subject or object has cardinality ONE and num results != 1, no
//    relations are emitted.
//  * If subject and object both have cardinality MANY but have different
//    numbers of results, no relations are emitted.
//
// Also, any annotation with cardinality ONE and num results != 1, or with
// cardinality MANY and num results != num_relations (as determined above), will
// be skipped (i.e. will not be emitted).
//
int ComputeNumRelations(
    const StringPiece& url,
    const RelationTemplate& rel_tmpl,
    const QueryResults& subject_results,
    const QueryResults& object_results,
    const vector<const QueryResults*>& annotation_results_vec,
    ErrorHandlingMode error_handling_mode,
    vector<bool>* skip_annotation_vec) {
  int num_relations = -1;
  bool skip_relation = false;

  if (rel_tmpl.subject_cardinality() == RelationTemplate::ONE) {
    if (subject_results.size() != 1) {
      MaybeLogCardinalityOneError(url, rel_tmpl, "subject",
                                  subject_results.size(), error_handling_mode);
      skip_relation = true;
    }
  } else {
    num_relations = subject_results.size();
  }

  if (rel_tmpl.object_cardinality() == RelationTemplate::ONE) {
    if (object_results.size() != 1) {
      MaybeLogCardinalityOneError(url, rel_tmpl, "object",
                                  object_results.size(), error_handling_mode);
      skip_relation = true;
    }
  } else {
    if (num_relations == -1) {
      num_relations = object_results.size();
    } else if (num_relations != object_results.size()) {
      LogCardinalityManyError(url, rel_tmpl, "object", object_results.size(),
                              num_relations, error_handling_mode);
      skip_relation = true;
    }
  }

  for (int i = 0; i < annotation_results_vec.size(); ++i) {
    const RelationTemplate::AnnotationTemplate& annotation_tmpl =
        rel_tmpl.annotation_tmpls(i);
    const int results_size = annotation_results_vec[i]->size();
    const string pseudonym =
        StrCat("annotation \"", annotation_tmpl.name(), "\"");
    bool skip_annotation = false;

    if (annotation_tmpl.value_cardinality() == RelationTemplate::ONE) {
      if (results_size != 1) {
        MaybeLogCardinalityOneError(url, rel_tmpl, pseudonym, results_size,
                                    error_handling_mode);
        skip_annotation = true;
      }
    } else {
      if (num_relations == -1) {
        num_relations = results_size;
      } else if (num_relations != results_size) {
        LogCardinalityManyError(url, rel_tmpl, pseudonym, results_size,
                                num_relations, error_handling_mode);
        skip_annotation = true;
      }
    }
    skip_annotation_vec->push_back(skip_annotation);
  }

  if (skip_relation) {
    return 0;
  } else if (num_relations == -1) {
    num_relations = 1;
  }
  return num_relations;
}

}  // namespace

// Returns the QueryResults* corresponding to 'key'.
// Evaluates the query and populates QueryResults if needed.
// 'key' should be a subject, object, or annotation value reference from some
// RelationTemplate.
const QueryResults* XpafParser::GetQueryResults(
    const StringPiece& url, const QueryRunner& query_runner, const string& key,
    QueryResultsCache* cache) const {
  DCHECK(!HasPrefixString(key, "/"))
      << "Inlined query should have been converted to a reference: " << key;

  const QueryResults* cached_results = cache->Get(key);
  if (cached_results != NULL) {
    return cached_results;
  }

  QueryResults* results = new QueryResults();

  QueryInfoMap::const_iterator info_it = query_info_map_.find(key);
  DCHECK(info_it != query_info_map_.end()) << key;
  const QueryInfo* query_info = info_it->second;

  if (query_info->query_def != NULL) {
    DCHECK(query_info->query_group_def == NULL);
    query_runner.RunStandaloneQuery(*query_info->query_def, results);
  } else if (query_info->query_group_def != NULL) {
    const QueryGroupDef& query_group_def = *query_info->query_group_def;
    vector<QueryResults*> results_vec(query_group_def.query_defs_size());
    for (int i = 0; i < results_vec.size(); ++i) {
      const string curr_ref =
          RefFromQueryName(GetGroupedQueryName(query_group_def,
                                               query_group_def.query_defs(i)));
      if (curr_ref != key) {
        QueryResults* curr_results = new QueryResults();
        results_vec[i] = curr_results;
        cache->Insert(curr_ref, curr_results);
      } else {
        results_vec[i] = results;  // cached below
      }
    }
    query_runner.RunGroupedQueries(query_group_def, &results_vec);
  } else if (!IsQueryRef(key)) {
    results->push_back(make_pair(key, true));
  } else if (key == "%url%") {
    results->push_back(make_pair(url.as_string(), true));
  } else {
    LOG(FATAL) << key;
  }

  cache->Insert(key, results);
  return results;
}

void XpafParser::Parse(const StringPiece& url,
                       const XPathWrapper& xpath_wrapper,
                       ParserOutput* output) const {
  CHECK(initialized_) << kForgotInitError;
  VLOG(1) << "XpafParser[" << ParserName() << "]::Parse(" << url << ")";
  DCHECK(ShouldParse(url)) << url;

  // First, create a QueryResultsCache and initialize QueryRunner.
  QueryResultsCache cache;
  const QueryRunner query_runner(url, xpath_wrapper,
                                 parse_options_.error_handling_mode);

  for (int i = 0; i < parser_def_.relation_tmpls_size(); ++i) {
    const RelationTemplate& rel_tmpl = parser_def_.relation_tmpls(i);
    if (rel_tmpl.has_url_regexp() &&
        !RE2::PartialMatch(re2::StringPiece(url.data(), url.size()),
                           rel_tmpl.url_regexp())) {
      continue;
    }

    VLOG(1) << "Processing template:\n" << rel_tmpl.DebugString();
    const QueryResults& subject_results =
        *GetQueryResults(url, query_runner, rel_tmpl.subject(), &cache);
    const QueryResults& object_results =
        *GetQueryResults(url, query_runner, rel_tmpl.object(), &cache);

    vector<const QueryResults*> annotation_results_vec;
    for (int j = 0; j < rel_tmpl.annotation_tmpls_size(); ++j) {
      annotation_results_vec.push_back(
          GetQueryResults(url, query_runner,
                          rel_tmpl.annotation_tmpls(j).value(), &cache));
    }

    vector<bool> skip_annotation_vec;
    const int num_relations =
        ComputeNumRelations(url, rel_tmpl, subject_results, object_results,
                            annotation_results_vec,
                            parse_options_.error_handling_mode,
                            &skip_annotation_vec);

    for (int j = 0; j < num_relations; ++j) {
      const int subject_idx =
          rel_tmpl.subject_cardinality() == RelationTemplate::MANY ? j : 0;
      const int object_idx =
          rel_tmpl.object_cardinality() == RelationTemplate::MANY ? j : 0;
      if (!subject_results[subject_idx].second ||
          !object_results[object_idx].second) {
        continue;
      }

      Relation* rel = output->add_relations();
      // TODO(sadovsky): Consider not setting these fields if string is empty.
      rel->set_subject(subject_results[subject_idx].first);
      rel->set_predicate(rel_tmpl.predicate());
      rel->set_object(object_results[object_idx].first);
      if (rel_tmpl.has_userdata()) {
        rel->set_userdata(rel_tmpl.userdata());
      }

      VLOG(1) << "Relation[" << ParserName() << "]: '" << rel->subject()
              << "', '" << rel->predicate() << "', '" << rel->object() << "'";

      for (int k = 0; k < annotation_results_vec.size(); ++k) {
        if (skip_annotation_vec[k]) {
          // This annotation has the wrong number of results, so we skip it.
          // See ComputeNumRelations() implementation for more information.
          continue;
        }
        const int annotation_idx =
            rel_tmpl.annotation_tmpls(k).value_cardinality() ==
            RelationTemplate::MANY ? j : 0;
        const QueryResults& annotation_results = *annotation_results_vec[k];
        if (!annotation_results[annotation_idx].second) {
          continue;
        }

        Relation::Annotation* annotation = rel->add_annotations();
        annotation->set_name(rel_tmpl.annotation_tmpls(k).name());
        annotation->set_value(annotation_results[annotation_idx].first);
      }
    }
  }
}

}  // namespace xpaf
