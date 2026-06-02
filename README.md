# Simple JVM & Dalvik VM

教學用的精簡 Java 虛擬機 (JVM) 與 Dalvik 虛擬機 (DVM),以 C 撰寫,給入門班理解 VM 如何
解析 class / dex 檔並執行 bytecode。刻意「夠用就好」:以整數運算為主,不追求完整規格。

> 原作者: Chun-Yu Wang (wicanr2@gmail.com), 2013。本版加上跨平台 build、回歸測試與 repo 整理。

## 專案結構

```
simple_jvm/    JVM: 解析 .class → 跑 <init> bytecode
simple_dvm/    DVM: 解析 .dex   → 跑 Dalvik bytecode
test/          golden 回歸測試 (run_golden.sh + golden/)
Foo1.java      範例程式; Foo1.class / Foo1.dex 為其編譯產物 (測試 fixture)
lib/           smali / baksmali / dx (Windows 端重新產生 dex 的工具)
Makefile       build 規則
Dockerfile     編譯環境 (gcc:13)
build.sh       在 docker 內跑 make 的包裝
```

兩個 VM 各自獨立、自包,parser 依 concern 一檔 (constant_pool / method / field /
string_ids / type_ids …),方便逐塊閱讀。詳見 `CONTEXT.md` 的術語表。

## Build (docker-first)

```bash
./build.sh          # build simple_jvm + simple_dvm → build/
./build.sh debug    # 帶 -DSIMPLE_*_DEBUG 的 debug 版
./build.sh clean
```

`build.sh` 會以目前使用者身分在 `gcc:13` 容器內跑 `make`,不污染系統環境。
若環境已有 gcc/make,也可直接 `make`。

## 執行

```bash
build/simple_jvm Foo1.class
build/simple_dvm Foo1.dex
```

## 測試

```bash
./build.sh test     # build 後跑 golden 回歸測試
```

測試以固定亂數種子 (`SVM_SEED`) 跑兩個 VM,比對輸出與 `test/golden/` 下的已知良好輸出。
**刻意**改動行為後要更新基準:

```bash
UPDATE=1 ./test/run_golden.sh   # (需先 make all, 或在 docker 內)
```

### 為什麼需要 SVM_SEED

`java.lang.Math.random` 用 `srand(time(0))`,輸出本來每次不同,無法回歸測試。
設環境變數 `SVM_SEED=<n>` 可固定種子讓輸出可重現;未設時維持原本的 `time(0)` 行為。

## 已知限制 (教學取捨)

- 以整數運算為主,型別系統不完整。
- 固定大小的 pool / register bank (例: `regs[32]`、`utf8CP[200]`),未做動態擴張。
- 僅實作範例程式用到的 bytecode 與 `java.lang.*` 子集。
