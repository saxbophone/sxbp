#!/bin/bash
# 
# Script for updating the project logo
# The first argument is the path to the sxp cli program.
# The second argument is the file to write the PBM output to.
# The third is the message to use for the logo data
# (of format "saxbospiral vX.Y.Z").
# 
echo "Generating logo";
./"$1" -pgrS "$3" -o "$2";
