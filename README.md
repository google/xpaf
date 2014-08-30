XPath-based Parsing Framework (XPaF)
====================================

XPath-based Parsing Framework (XPaF) is a simple, fast, open-source parsing
framework that makes it easy to extract relations (subject-predicate-object
triples) from HTML and XML documents.

Documentation is available [here](http://htmlpreview.github.io/?https://github.com/google/xpaf/blob/master/doc/index.html).

Copyright 2011 Google Inc. All Rights Reserved.


Installation instructions
-------------------------

1. Install libraries (see platform-specific instructions below)

2. Build xpaf, run tests, and install

   ```
   ./autogen.sh
   ./configure
   make
   make check
   ```

3. Clean up

   ```
   make clean
   make maintainer-clean
   ```


Unix installation instructions
------------------------------

1. Install autotools

   ```
   apt-get install autoconf automake libtool
   ```

2. Install libraries

   ```
   apt-get install gflags libgtest-dev libprotobuf-dev libxml2 protobuf-compiler
   ```

3. Install re2 library

   ```
   hg clone https://re2.googlecode.com/hg re2
   cd re2
   make install
   ```

4. Run ldconfig to set up symlinks for libraries

   ```
   ldconfig -n /usr/lib /usr/local/lib
   ```


OS X installation instructions
------------------------------

1. Install Homebrew (http://brew.sh/)

2. Install libraries

   ```
   brew install automake libtool gflags libxml2 protobuf re2
   ```

3. Install gtest library

   ```
   curl -O https://googletest.googlecode.com/files/gtest-1.7.0.zip
   unzip gtest-1.7.0.zip
   cd gtest-1.7.0
   ./configure
   make
   cp -a include/gtest /usr/local/include
   cp -a lib/.libs/* /usr/local/lib/
   ```
