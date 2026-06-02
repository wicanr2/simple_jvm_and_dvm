#!/usr/bin/env bash
#
# End-to-end 測試: 從已 build 的 binary, 跑完整流程並驗證多項不變量。
# 由 `make e2e` (= all + debug 後) 在 docker 內呼叫。
#
#   檢查項目:
#     1. release / debug binaries 都有產出
#     2. 兩個 VM 以 exit 0 正常結束
#     3. golden 回歸 (test/run_golden.sh)
#     4. cross-VM 一致性: JVM 與 DVM 對同一支 Foo1 產生相同執行結果
#     5. debug build 可正常執行 (smoke)
#     6. 固定 seed 下可重現 (determinism)
#
set -uo pipefail
cd "$(dirname "$0")/.."

SEED=42
TMP=test/.tmp
mkdir -p "$TMP"
pass=0; fail=0

check() { # $1: rc, $2: desc
    if [ "$1" -eq 0 ]; then echo "PASS  $2"; pass=$((pass+1)); else echo "FAIL  $2"; fail=$((fail+1)); fi
}

# 1. binaries exist
{ [ -x build/simple_jvm ] && [ -x build/simple_dvm ]; }; check $? "release binaries built"
{ [ -x build/simple_jvm_debug ] && [ -x build/simple_dvm_debug ]; }; check $? "debug binaries built"

# 2. run release, exit 0
SVM_SEED=$SEED build/simple_jvm Foo1.class > "$TMP/jvm.out" 2>&1; jrc=$?
SVM_SEED=$SEED build/simple_dvm Foo1.dex   > "$TMP/dvm.out" 2>&1; drc=$?
check $jrc "simple_jvm exits 0"
check $drc "simple_dvm exits 0"

# 3. golden regression
./test/run_golden.sh > "$TMP/golden.log" 2>&1; check $? "golden regression"

# 4. cross-VM consistency: 共同執行區塊 (HelloWorld .. Foo Test By WJY) 相等
extract() { awk '/^HelloWorld$/{f=1} f{print} /^Foo Test By WJY$/{f=0}' "$1"; }
extract "$TMP/jvm.out" > "$TMP/jvm.exec"
extract "$TMP/dvm.out" > "$TMP/dvm.exec"
{ [ -s "$TMP/jvm.exec" ] && diff -q "$TMP/jvm.exec" "$TMP/dvm.exec" >/dev/null; }; check $? "JVM == DVM execution result"

# 5. debug build smoke
SVM_SEED=$SEED build/simple_jvm_debug Foo1.class >/dev/null 2>&1; check $? "debug jvm runs"
SVM_SEED=$SEED build/simple_dvm_debug Foo1.dex   >/dev/null 2>&1; check $? "debug dvm runs"

# 6. determinism
SVM_SEED=$SEED build/simple_jvm Foo1.class > "$TMP/jvm.r2" 2>&1
SVM_SEED=$SEED build/simple_dvm Foo1.dex   > "$TMP/dvm.r2" 2>&1
{ diff -q "$TMP/jvm.out" "$TMP/jvm.r2" >/dev/null && diff -q "$TMP/dvm.out" "$TMP/dvm.r2" >/dev/null; }; check $? "deterministic re-run"

echo "----------------------------------------"
echo "e2e: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
