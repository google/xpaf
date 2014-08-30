#!/bin/bash

# Runs all parsers on each http file and writes the resulting out file. Reads
# from and writes to the current directory. Use with caution.

for f in `ls ./*.http`; do
  g=`echo $f | sed 's/http/out/'`;
  ./parse_tool \
      --parser_defs_glob=./*.xpd \
      --input_file_path=$f 1>$g 2>/dev/null;
  echo $g;
done
