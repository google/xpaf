# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Process this file with autoconf to produce configure.
# Note: You should probably just run ./autogen.sh, as per the README file.

AC_PREREQ(2.59)

AC_INIT(xpaf, 0.0, xpaf-discuss@googlegroups.com)

# NOTE: This value must be updated for every release.
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
SO_VERSION=0:0:0  # current:revision:age

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([build-aux/config.h])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AM_CONDITIONAL(GCC, test "$GCC" = yes)

AC_CANONICAL_HOST

AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(SO_VERSION)

# Check for various headers and libraries.
AC_HEADER_STDC

PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= 2.7.6)
# For pkg-config <0.24, PKG_CHECK_MODULES() doesn't call AC_SUBST, so we do it
# ourselves.
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

AC_CHECK_LIB(re2, main,
             AC_SUBST(LIBRE2_LIBS, "-lre2"),
             AC_MSG_FAILURE([Missing library re2]))
AC_CHECK_LIB(protobuf, main,
             AC_SUBST(LIBPROTOBUF_LIBS, "-lprotobuf"),
             AC_MSG_FAILURE([Missing library protobuf]),
             [-lpthread])
AC_CHECK_LIB(gflags, main,
             AC_SUBST(LIBGFLAGS_LIBS, "-lgflags"),
             AC_MSG_FAILURE([Missing library gflags]))
AC_CHECK_LIB(gtest, main,
             AC_SUBST(LIBGTEST_LIBS, "-lgtest"),
             AC_MSG_FAILURE([Missing library gtest]))

AC_CHECK_PROG(PROTOC, protoc, true, false)
if test "x${PROTOC}" != "xtrue"; then
  echo "ERROR: Missing program protoc";
  exit 1;
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
