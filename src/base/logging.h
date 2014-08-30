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

// Simplified version of Google's logging.

#ifndef XPAF_BASE_LOGGING_H_
#define XPAF_BASE_LOGGING_H_

#include "base/macros.h"
#include "base/stl_decl.h"

namespace xpaf {

// Always-on checking.
#define CHECK(x) if (x) {} else LogMessageFatal(__FILE__, __LINE__).stream() \
                                    << "Check failed: " #x
#define CHECK_LT(x, y) CHECK((x) < (y))
#define CHECK_GT(x, y) CHECK((x) > (y))
#define CHECK_LE(x, y) CHECK((x) <= (y))
#define CHECK_GE(x, y) CHECK((x) >= (y))
#define CHECK_EQ(x, y) CHECK((x) == (y))
#define CHECK_NE(x, y) CHECK((x) != (y))
#define CHECK_NOTNULL(x) CHECK((x) != NULL)

// Debug-only checking.
#ifndef NDEBUG

#define DCHECK(condition) CHECK(condition)
#define DCHECK_EQ(val1, val2) CHECK_EQ(val1, val2)
#define DCHECK_NE(val1, val2) CHECK_NE(val1, val2)
#define DCHECK_LE(val1, val2) CHECK_LE(val1, val2)
#define DCHECK_LT(val1, val2) CHECK_LT(val1, val2)
#define DCHECK_GE(val1, val2) CHECK_GE(val1, val2)
#define DCHECK_GT(val1, val2) CHECK_GT(val1, val2)

#else

// Note: We define the macros below such that in NDEBUG mode,
//  * values used only in DCHECKs aren't considered unused;
//  * the compiler requires that DCHECK statements end in a semicolon; and
//  * DCHECK(...) << "foo" compiles.

#define DCHECK(condition) while (false) CHECK(condition)
#define DCHECK_EQ(val1, val2) while (false) CHECK_EQ(val1, val2)
#define DCHECK_NE(val1, val2) while (false) CHECK_NE(val1, val2)
#define DCHECK_LE(val1, val2) while (false) CHECK_LE(val1, val2)
#define DCHECK_LT(val1, val2) while (false) CHECK_LT(val1, val2)
#define DCHECK_GE(val1, val2) while (false) CHECK_GE(val1, val2)
#define DCHECK_GT(val1, val2) while (false) CHECK_GT(val1, val2)

#endif  // NDEBUG

#define LOG_INFO LogMessage(__FILE__, __LINE__)
#define LOG_ERROR LOG_INFO
#define LOG_WARNING LOG_INFO
#define LOG_FATAL LogMessageFatal(__FILE__, __LINE__)
#define LOG_QFATAL LOG_FATAL

#define VLOG(x) if ((x) > 0) {} else LOG_INFO.stream()

#ifdef NDEBUG
#define DEBUG_MODE 0
#define LOG_DFATAL LOG_ERROR
#else
#define DEBUG_MODE 1
#define LOG_DFATAL LOG_FATAL
#endif  // NDEBUG

#define LOG(severity) LOG_ ## severity.stream()

class LogMessage {
 public:
  LogMessage(const char* file, int line) {
    std::cerr << file << ":" << line << ": ";
  }
  ~LogMessage() { std::cerr << "\n"; }
  std::ostream& stream() { return std::cerr; }

 private:
  DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

class LogMessageFatal : public LogMessage {
 public:
  LogMessageFatal(const char* file, int line) : LogMessage(file, line) {}
  ~LogMessageFatal() {
    std::cerr << "\n";
    abort();
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(LogMessageFatal);
};

}  // namespace xpaf

#endif  // XPAF_BASE_LOGGING_H_
