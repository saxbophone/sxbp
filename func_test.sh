#!/bin/bash
# 
# Functional test script.
# Generates a new logo and compares with the current one.
# The first argument is the path to the sxp cli program.
# The second argument is the message to use for the file (see build_logo.sh)
# 
./build_logo.sh "$1" "test_sxbp.pbm" "$2" && \
diff "sxbp.pbm" "test_sxbp.pbm" && \
rm "test_sxbp.pbm";
