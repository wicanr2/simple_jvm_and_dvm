# Refactor 報告 — Simple JVM & Dalvik VM

> 範圍: 基礎建設 + 結構改善 (不含共用抽象層)。原則: 先建立可重現的 pass/fail 訊號,
> 再動邏輯,每步以 golden 回歸驗證。日期: 2026-06-02。

## 摘要

把一個 2013 年、只能在 Windows (mingw) build、commit 了一堆 build 產物、無測試的
教學專案,整理成:**docker-first 跨平台 build + 確定性 e2e 回歸測試 + 乾淨的 repo**,
且不破壞既有執行語意 (golden 與 cross-VM 一致性均通過)。

## 改了什麼

### 1. 跨平台 build (docker-first)

| 檔案 | 作用 |
|---|---|
| `Makefile` | `make` / `debug` / `test` / `e2e` / `clean`,wildcard 抓 source |
| `Dockerfile` | `gcc:13` 編譯環境 |
| `build.sh` | 在 docker 內以目前使用者身分跑 `make`,不留 root 檔、不污染系統 |

原本只有 `make_simple_*.bat` (mingw32, Windows only)。現在 Linux/macOS 皆可 build。

### 2. 確定性 (測試前提)

`java.lang.Math.random` 用 `srand(time(0))`,輸出每次不同,無法回歸。
新增 `SVM_SEED` 環境變數: 設定即用固定 seed (只 seed 一次),未設維持原 `time(0)` 行為。
另修掉 DVM 中一行裸印 heap 指標位址的 debug `printf` (ASLR 造成非確定),納入 verbose 守門。

### 3. 正確性修正 (golden 驗證無回歸)

- DVM `typedef short u2` → `unsigned short` (DEX 規格 u2 為 unsigned)。
- 補 `<time.h>` (原先 `time()` 隱式宣告)。
- `static byteCode_size` → `static int byteCode_size` (implicit-int)。

### 4. Golden + E2E 測試

| 檔案 | 作用 |
|---|---|
| `test/run_golden.sh` | 固定 seed 跑 VM,比對 `test/golden/`;`UPDATE=1` 重新產生基準 |
| `test/run_e2e.sh` | 9 項不變量檢查 (見下) |
| `test/golden/*.txt` | 已知良好輸出 |

### 5. Repo 衛生

- untrack 4 個 `.exe` + 6 個 log/dasm 產物 (保留磁碟檔)。
- 新增 `.gitignore`、`README.md`、`CONTEXT.md` (domain glossary)。

## E2E 結果

`./build.sh e2e` → clean → build release+debug → 跑 `test/run_e2e.sh` (docker 內):

```
PASS  release binaries built
PASS  debug binaries built
PASS  simple_jvm exits 0
PASS  simple_dvm exits 0
PASS  golden regression
PASS  JVM == DVM execution result      ← 兩 VM 對同一支 Foo1 產生相同執行結果
PASS  debug jvm runs
PASS  debug dvm runs
PASS  deterministic re-run
----------------------------------------
e2e: 9 passed, 0 failed
```

最關鍵的不變量是 **JVM == DVM execution result**: 在 `SVM_SEED=42` 下,JVM 與 DVM 對
`Foo1` 從 `HelloWorld` 到 `Foo Test By WJY` 的執行輸出完全相同,證明兩個 VM 實作相同語意。

## 如何使用

```bash
./build.sh          # build
./build.sh test     # golden 回歸
./build.sh e2e      # 完整 e2e
build/simple_jvm Foo1.class
build/simple_dvm Foo1.dex
```

## Phase E 進度

### E-1 debug log 收斂 (完成)
JVM `bytecodes.c` / `java_lib.c` 共 49 處純 printf 的 `#if SIMPLE_JVM_DEBUG` 區塊改用
`JVM_LOG()` 巨集 (release no-op, debug = printf);保留 2 個含函式呼叫/條件的區塊。
golden 強化: 新增 jvm/dvm debug build 案例 (release+debug 共 4 份 golden)。

### E-3 清 warning + demo 移位 (完成)
release / debug build 皆 **0 warning** (原 85 個)。重點:
- **真 bug**: `simple_dvm_dex_parser.c` `memset(dex,0,sizeof(dex))` → `sizeof(*dex)`
  (原只清了指標大小 8 bytes,靠全域變數預設歸零才沒爆)。
- 16 個 `-Wreturn-type` (缺 return 的 UB 風險) 補 `return 0;`。
- sign-compare / format / unused / pointer-sign 逐一修正,皆不改變輸出 (golden 驗證)。
- `test/{a,b}.cpp`、`test/main.c` (與 VM 無關的 demo) 移到 `examples/c_cpp_linkage/`。

### E-2 JVM 全域狀態 → context struct (完成)
原本散落的 6 個全域 pool (`simpleConstantPool`/`simpleMethodPool`/`stackFrame`/
`localVariables` …) 收進單一 `JvmContext`,由 `main` 以 `static JvmContext jvm` 擁有、
以指標顯式傳遞 (對齊 DVM 的 `DexFileFormat*`/`simple_dalvik_vm*` 風格),**移除所有全域
可變狀態與 `extern` 宣告** (剩 0 個)。

- `opCodeFunc` 簽章追加 `JvmContext *ctx`;35 個 op、各 parser、`executeMethod`、
  `free_pools`、`parseJavaClassFile` 全部串接 ctx。
- 過渡設計: op 仍同時收 `stack`/`p` 參數 (= `&ctx->stack` / `&ctx->constant_pool`),
  與 ctx 欄位重複,之後可再收斂 — 換取本次 op body 幾乎零變動、好 review。
- **驗證限制**: golden 涵蓋 Foo1 執行路徑 (含 debug build 28KB 輸出, 會跑到 invoke ops);
  未被 Foo1 觸發的 op 由 compiler 型別檢查 + 別名設計保證 (body 不變) 把關。

## 其他可選後續

| 項目 | 風險 | 說明 |
|---|---|---|
| 從 `Foo1.java` 全鏈重生 class/dex | 低 | 需在 docker 裝 jdk + dx,可選 |
