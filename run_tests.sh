#!/bin/bash
# C3 test script
# Apache v2.0 - Copyright 2017 - rkx1209

c3 -v > /dev/null
if [ $? != 0 ]; then
  echo "Cannnot find c3"
  exit 1
fi

TESTS_TOTAL=0
TESTS_SUCCESS=0
TESTS_FAILED=0

. ./test_template.sh

TEST_DIR="test"
# Run all tests
if [ -f "$1" -a -x "$1" ]; then
  # test script is specified
  BASE=`dirname $1`
  FILE=`basename $1`
  cd $BASE
  . ./$FILE
else
  cd $TEST_DIR || die "test/ doesn't exist"
  do_tests_recusive() {
    for i in "$1"/*; do
      if [ -d "$i" ]; then
        cd ${i}
        do_tests_recusive "."
        cd ..
      elif [ -f "$i" ]; then
        if [ -x "$i" ]; then
          NAME=`basename $i`
          TEST_NAME=${NAME}
          . ./${i}
        fi
      fi
    done
  }
  do_tests_recusive "."
fi

print_report
