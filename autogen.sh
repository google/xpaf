#!/bin/sh

set -ex

rm -rf autom4te.cache
mkdir -p m4

# Use glibtoolize if available (e.g. on Mac), libtoolize otherwise.
if which glibtoolize; then
  LIBTOOLIZE=glibtoolize
else
  LIBTOOLIZE=libtoolize
fi

aclocal --force -I m4
grep -q '^[^#]*AC_PROG_LIBTOOL' configure.ac && $LIBTOOLIZE -c -f
autoconf -f -W all,no-obsolete
autoheader -f -W all
automake -a -c -f -W all

exit 0
