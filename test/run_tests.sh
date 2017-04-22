#!/bin/sh
# C3 test script
# Apache v2.0 - Copyright 2017 - rkx1209

c3 -v
if [ $? != 0 ]; then
  echo "Cannnot find c3"
  exit 1
fi
