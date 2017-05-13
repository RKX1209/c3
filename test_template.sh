#!/bin/false
# C3 test script template
# Apache v2.0 - Copyright 2017 - rkx1209

DIFF=diff
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

run_test() {
  [ -z "${C3}" ] && C3=$(which c3)
  if [ -z "${C3}" ]; then
    echo "ERROR: Cannnot find c3 program in PATH"
    exit 1
  fi
  if [ -z "${TEST_NAME}" ]; then
    TEST_NAME=$(basename "$0" | sed 's/\.sh$//')
  fi

  RDIR="/tmp/c3-test"
  mkdir -p ${RDIR} || exit 1
  TMP_DIR="`mktemp -d "${RDIR}/${TEST_NAME}-XXXXXX"`"
  if [ $? != 0 ]; then
    echo "Cannnot create test directory"
    exit 1
  fi
  TMP_NAM="${TMP_DIR}/nam" # test name
  TMP_ERR="${TMP_DIR}/err" # stderr
  TMP_OUT="${TMP_DIR}/out" # stdout
  TMP_EXP="${TMP_DIR}/exp" # expected
  TMP_ODF="${TMP_DIR}/odf" # output diff

  : > "${TMP_OUT}"
  cat > "$TMP_NAM" << __EOF__
  $TEST_NAME / $NAME
__EOF__
  printf "%s\n" "${EXPECT}" > ${TMP_EXP}
  C3ARGS="-D ${FILE} > ${TMP_OUT} 2> ${TMP_ERR}"
  C3CMD="${C3} ${C3ARGS}"
  echo "${C3CMD}"
  eval "${C3CMD}"
  CODE=$?
  TESTS_TOTAL=$(( TESTS_TOTAL + 1 ))
  ${DIFF} -u "${TMP_EXP}" "${TMP_OUT}" > "${TMP_ODF}"
  OUT_CODE=0
  [ -s "${TMP_ODF}" ] && OUT_CODE=1

  if [ ${CODE} -ne 0 ]; then
    test_failed "c3 crashed"
  elif [ ${OUT_CODE} -ne 0 ]; then
    test_failed "unexpected ${TMP_ERR}"
  else
    test_success
  fi
  #echo "run_test $OUT_CODE"
  return $OUT_CODE
}

test_success() {
  TESTS_SUCCESS=$(( TESTS_SUCCESS + 1 ))
}

test_failed() {
  print_failed "${*}"
  TESTS_FAILED=$(( TESTS_FAILED + 1 ))
  if [ ${FAIL_OVER} -eq 1 ]; then
    exit 1
  fi
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
