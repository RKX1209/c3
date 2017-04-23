#!/bin/false
# C3 test script template
# Apache v2.0 - Copyright 2017 - rkx1209

die() {
  echo "$1"
  exit 1
}

print_success() {
  printf "%b" "${NL}\033[32m[${*}]\033[0m\n"
}

print_failed() {
  printf "%b" "${NL}\033[31m[${*}]\033[0m\n"
}

print_label() {
  printf "\033[35m%s \033[0m" $@
}

print_report() {
  echo "=== Report ==="
  printf "    SUCCESS"
  if [ "${TESTS_SUCCESS}" -gt 0 ]; then
    print_success "${TESTS_SUCCESS}"
  else
    print_failed "${TESTS_SUCCESS}"
  fi
  printf "    FAILED"
  if [ "${TESTS_FAILED}" -gt 0 ]; then
    print_failed "${TESTS_FAILED}"
  else
    print_failed 0
  fi
  printf "    TOTAL"
  print_label "[${TESTS_TOTAL}]"
  echo ""
}
