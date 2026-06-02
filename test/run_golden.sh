#!/usr/bin/env bash
#
# Golden 回歸測試: 用固定 seed 跑 VM, 比對輸出與 test/golden/ 下的已知良好輸出。
#
#   ./test/run_golden.sh          # 跑測試 (PASS/FAIL)
#   UPDATE=1 ./test/run_golden.sh # 重新產生 golden (改動行為後刻意更新時用)
#
# 由 `make test` 在 docker 內呼叫; 也可單獨執行 (需先 make all)。
#
set -uo pipefail
cd "$(dirname "$0")/.."

SEED=42
GOLDEN=test/golden
TMP=test/.tmp
mkdir -p "$GOLDEN" "$TMP"

rc=0

run_one() {
    local name="$1"; shift
    local golden="$GOLDEN/$name.txt"
    local out="$TMP/$name.out"

    # debug 案例: binary 不存在 (未 make debug) 時跳過
    if [ ! -x "$1" ]; then
        echo "SKIP  $name  ($1 未 build)"
        return 0
    fi

    SVM_SEED=$SEED "$@" > "$out" 2>&1

    if [ "${UPDATE:-0}" = "1" ]; then
        cp "$out" "$golden"
        echo "UPDATED $golden"
        return 0
    fi
    if [ ! -f "$golden" ]; then
        echo "MISSING golden: $golden  (先跑 UPDATE=1 產生)"
        rc=1
        return 0
    fi
    if diff -u "$golden" "$out" > "$TMP/$name.diff"; then
        echo "PASS  $name"
    else
        echo "FAIL  $name  (diff: $TMP/$name.diff)"
        head -40 "$TMP/$name.diff"
        rc=1
    fi
}

run_one jvm_Foo1       build/simple_jvm       Foo1.class
run_one dvm_Foo1       build/simple_dvm       Foo1.dex
run_one jvm_GEMM       build/simple_jvm       GEMM.class
run_one dvm_GEMM       build/simple_dvm       GEMMDvm.dex
run_one jvm_Foo1_debug build/simple_jvm_debug Foo1.class
run_one dvm_Foo1_debug build/simple_dvm_debug Foo1.dex

if [ "$rc" -eq 0 ]; then
    echo "---- all golden tests passed ----"
else
    echo "---- golden tests FAILED ----"
fi
exit $rc
