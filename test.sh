#!/usr/bin/env bash
set -euo pipefail

BIN=./bin/psdbg
SELFPID=$$
PASS=0
FAIL=0

ok() { PASS=$((PASS+1)); echo "  ok  $1"; }
no() { FAIL=$((FAIL+1)); echo "  no  $1"; }

run() { "$BIN" "$@" 2>/dev/null; }

[ -x "$BIN" ] || { echo "building psdbg..."; make -s 2>/dev/null; }

echo
echo "--- help / version ---"

run --help | grep -q "Usage:"        && ok "--help shows usage"       || no "--help shows usage"
run --version | grep -q psdbg        && ok "--version shows version"  || no "--version shows version"

echo
echo "--- list mode ---"

LIST="$(run)"
JSON="$(run --json)"
CSV="$(run --csv)"
RAW="$(run --raw)"

[ "$(echo "$LIST" | wc -l)" -gt 2 ]                         && ok "list has output"       || no "list has output"
echo "$LIST" | grep -q "PID"                                 && ok "list has header"       || no "list has header"
echo "$JSON" | grep -q "^\["                                 && ok "json starts with ["    || no "json starts with ["
echo "$JSON" | tail -1 | grep -q "\]"                        && ok "json ends with ]"       || no "json ends with ]"
echo "$CSV" | grep -q "^PID,USER,CPU,MEM"                    && ok "csv has header"        || no "csv has header"
echo "$RAW" | grep -q "$(printf '\t')"                       && ok "raw has tabs"           || no "raw has tabs"

echo
echo "--- tree mode ---"

TREE="$(run --tree)"
[ "$(echo "$TREE" | wc -l)" -gt 2 ]                          && ok "--tree has output"     || no "--tree has output"
echo "$TREE" | grep -qP "\(\d+\)$"                           && ok "--tree shows PIDs"     || no "--tree shows PIDs"

echo
echo "--- detail mode ---"

DETAIL="$(run "$SELFPID")"
echo "$DETAIL" | grep -q "PID:"                              && ok "detail shows PID"      || no "detail shows PID"
echo "$DETAIL" | grep -q "State:"                            && ok "detail shows state"    || no "detail shows state"

echo
echo "--- threaded modes ---"

THR="$(run --threads "$SELFPID")"
SCH="$(run --sched "$SELFPID")"
MEM="$(run --memory "$SELFPID")"
AFF="$(run --affinity "$SELFPID")"
NS="$(run --namespaces "$SELFPID")"
WHY="$(run --why "$SELFPID")"
LIM="$(run --limits "$SELFPID")"
ENV="$(run --environment "$SELFPID")"

echo "$THR" | grep -q "TID"                                  && ok "--threads"             || no "--threads"
echo "$THR" | grep -qP "^[ \t]*\d+"                          && ok "--threads has entries" || no "--threads has entries"
echo "$SCH" | grep -q "Policy:"                               && ok "--sched"               || no "--sched"
echo "$SCH" | grep -q "CS:"                                   && ok "--sched context switches" || no "--sched context switches"
echo "$MEM" | grep -q "Resident"                              && ok "--memory"              || no "--memory"
echo "$MEM" | grep -q "Anonymous\|File-backed\|Shared"       && ok "--memory breakdown"    || no "--memory breakdown"
echo "$AFF" | grep -q "Allowed"                               && ok "--affinity"            || no "--affinity"
echo "$NS" | grep -q "Namespace"                              && ok "--namespaces"          || no "--namespaces"
echo "$NS" | grep -qP "[0-9]{7,}"                             && ok "--namespaces has IDs"  || no "--namespaces has IDs"
echo "$WHY" | grep -q "CPU\|scheduler\|state"                 && ok "--why"                 || no "--why"
echo "$LIM" | grep -q "Resource"                              && ok "--limits"              || no "--limits"
[ "$(echo "$LIM" | wc -l)" -gt 4 ]                            && ok "--limits has entries"  || no "--limits has entries"
echo "$ENV" | grep -q "="                                     && ok "--environment"          || no "--environment"

echo
echo "--- error handling ---"

run --threads 2>/dev/null && no "--threads missing PID errors" || ok "--threads missing PID errors"
run --memory 2>/dev/null && no "--memory missing PID errors"   || ok "--memory missing PID errors"
run 99999999 2>/dev/null && no "invalid PID errors"            || ok "invalid PID errors"
run --bogus 2>/dev/null && no "bogus flag errors"              || ok "bogus flag errors"
run --environment 1 2>/dev/null && ok "PID 1 env succeeds"     || ok "PID 1 env blocked (permission)"

echo
echo "=== $((PASS+FAIL)) tests, $PASS passed, $FAIL failed ==="
exit $FAIL
