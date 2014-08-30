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

# Process this file with automake to produce Makefile.in.
# Note: You should probably just run ./autogen.sh, as per README file.

# TODO(sadovsky): For some reason "make install" puts headers in
# /usr/local/include/xpaf/src and /usr/local/include/xpaf/base/src/base, which
# seems inconsistent.

ACLOCAL_AMFLAGS = -I m4

# Note: We need /opt/local/include and /opt/local/lib for MacPorts.
AM_CXXFLAGS = -DNDEBUG -I$(top_srcdir)/src -I/opt/local/include
AM_CXXFLAGS += @LIBXML2_CFLAGS@

if GCC
# Enable some warnings.
AM_CXXFLAGS += -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare
endif

# Note: We try to only link what we need for each target.
# For example, libxpaf.la doesn't include gflags or gtest.
AM_LDFLAGS = -no-undefined -L/opt/local/lib -version-info @SO_VERSION@
AM_LDFLAGS += @LIBXML2_LIBS@ @LIBRE2_LIBS@ @LIBPROTOBUF_LIBS@

lib_LTLIBRARIES = libxpaf.la

# Note: We use "nodist" to strip the src/ dir, and define pkgincludebasedir so
# that relative dirs are maintained.

nobase_pkginclude_HEADERS =\
    src/document.h\
    src/xpaf_parser.h\
    src/xpaf_parser_master.h\
    src/xpath_wrapper.h

pkgincludebasedir = $(pkgincludedir)/base
nobase_pkgincludebase_HEADERS =\
    src/base/macros.h\
    src/base/scoped_ptr.h\
    src/base/stl_decl.h\
    src/base/stringpiece.h

# Install public proto files alongside headers.
nobase_pkginclude_HEADERS += $(protoc_inputs)

noinst_HEADERS =\
    src/base/callback.h\
    src/base/commandlineflags.h\
    src/base/file.h\
    src/base/integral_types.h\
    src/base/logging.h\
    src/base/stl_util.h\
    src/base/strutil.h\
    src/base/url.h\
    src/base/webutil.h\
    src/document.h\
    src/query_runner.h\
    src/util.h\
    src/xpaf_parser.h\
    src/xpaf_parser_master.h\
    src/xpath_wrapper.h

nodist_noinst_HEADERS =\
    src/parsed_document.pb.h\
    src/post_processing_ops.pb.h\
    src/xpaf_parser_def.pb.h

libxpaf_la_SOURCES =\
    src/base/file.cc\
    src/base/stringpiece.cc\
    src/base/strutil.cc\
    src/base/webutil.cc\
    src/query_runner.cc\
    src/util.cc\
    src/xpaf_parser.cc\
    src/xpaf_parser_master.cc\
    src/xpath_wrapper.cc

nodist_libxpaf_la_SOURCES =\
    src/parsed_document.pb.cc\
    src/post_processing_ops.pb.cc\
    src/xpaf_parser_def.pb.cc

protoc_inputs =\
    src/parsed_document.proto\
    src/post_processing_ops.proto\
    src/xpaf_parser_def.proto

protoc_outputs =\
    src/parsed_document.pb.cc\
    src/parsed_document.pb.h\
    src/post_processing_ops.pb.cc\
    src/post_processing_ops.pb.h\
    src/xpaf_parser_def.pb.cc\
    src/xpaf_parser_def.pb.h

# Ensure proto headers are built before we attempt to build other targets.

BUILT_SOURCES = $(protoc_outputs)

protoc_helper: $(protoc_inputs)
	protoc -I$(srcdir) --cpp_out=. $^
	touch protoc_helper

$(protoc_outputs): protoc_helper

bin_PROGRAMS = parse_tool
parse_tool_LDADD = libxpaf.la @LIBGFLAGS_LIBS@
parse_tool_SOURCES = src/parse_tool.cc


##############
# Benchmarks

noinst_PROGRAMS = re2_strstr_bm
re2_strstr_bm_LDADD = @LIBGFLAGS_LIBS@
re2_strstr_bm_SOURCES =\
    src/base/benchmark.h\
    src/base/benchmark.cc\
    src/testing/re2_strstr_bm.cc

noinst_PROGRAMS += xpaf_bm
xpaf_bm_LDADD = libxpaf.la @LIBGFLAGS_LIBS@
xpaf_bm_SOURCES =\
    src/base/benchmark.h\
    src/base/benchmark.cc\
    src/testing/xpaf_bm.cc


##################
# Tests and lint

TESTS =

TESTS += init_test
init_test_LDADD = libxpaf.la @LIBGFLAGS_LIBS@ @LIBGTEST_LIBS@
init_test_SOURCES =\
    src/testing/gtest_main.cc\
    src/testing/init_test.cc

TESTS += parse_test
parse_test_LDADD = libxpaf.la @LIBGFLAGS_LIBS@ @LIBGTEST_LIBS@
parse_test_SOURCES =\
    src/testing/gtest_main.cc\
    src/testing/parse_test.cc

noinst_PROGRAMS += $(TESTS)

lint:
	gjslint --nojsdoc --nobeep chrome/*.js doc/*.js


#########
# Clean

maintainer-clean-local:
	rm -rf m4
	rm -rf build-aux

# Note: Assertions inside EXPECT_DEATH tests trigger core dumps, so we delete
# the core file (if it exists) on 'make clean'.

CLEANFILES = $(pkgconfig_DATA) protoc_helper core

MAINTAINERCLEANFILES =\
    Makefile.in\
    aclocal.m4\
    configure


################
# Distribution

EXTRA_DIST =\
    autogen.sh\
    libtool\
    AUTHORS\
    CONTRIBUTORS\
    README\
    LICENSE\
    $(protoc_inputs)\
    src/testing/*.sh\
    src/testing/init_test_data\
    src/testing/parse_test_data\
    src/testing/xpaf_bm_data\
    chrome

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
