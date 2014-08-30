#!/bin/bash

# Uses wget to fetch the given url and writes a file in the format expected by
# parse_tool.

url="$1"
file="$2"

body=`wget --save-headers -O - $url`

echo "${url}" > $file
echo "${body}" >> $file
