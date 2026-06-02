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
GEMM.java      矩陣乘法範例 (陣列 + 巢狀迴圈); GEMM.class 為測試 fixture
examples/      與 VM 無關的小範例 (c_cpp_linkage)
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
build/simple_jvm GEMM.class
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

## GEMM 範例 — 在 Simple JVM 上跑矩陣乘法

`GEMM.java` 是一個 **General Matrix Multiply** (C = A × B) 範例,示範這台 VM 如何執行
**陣列 + 巢狀迴圈** 的 bytecode。為了支援它,VM 新增了陣列與分支類 opcode (見下)。

```java
class GEMM {
    GEMM() {                       // 邏輯放在建構子: 本 VM 執行 <init>
        int n = 2;
        int[] a = new int[4];      // A = [[1,2],[3,4]]  (row-major 1D 陣列)
        int[] b = new int[4];      // B = [[5,6],[7,8]]
        int[] c = new int[4];
        a[0]=1; a[1]=2; a[2]=3; a[3]=4;
        b[0]=5; b[1]=6; b[2]=7; b[3]=8;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++)
                    sum = sum + a[i*n+k] * b[k*n+j];
                c[i*n+j] = sum;
            }
        System.out.println("GEMM 2x2 : C = A x B");
        System.out.println("c[0] = " + c[0]);   // 19
        System.out.println("c[1] = " + c[1]);   // 22
        System.out.println("c[2] = " + c[2]);   // 43
        System.out.println("c[3] = " + c[3]);   // 50
    }
}
```

執行:

```bash
build/simple_jvm GEMM.class
# GEMM 2x2 : C = A x B
# c[0] = 19   c[1] = 22   c[2] = 43   c[3] = 50   (與真實 JVM 一致)
```

### 它怎麼運作

**1. 編譯管線**

```
GEMM.java --(javac, JDK 8)--> GEMM.class --(simple_jvm)--> 執行 <init> 的 bytecode
```

用 **JDK 8** 編譯是刻意的: JDK 9+ 會把字串串接 (`"c[0] = " + c[0]`) 編成 `invokedynamic`
(StringConcatFactory),本 VM 不支援;JDK 8 編成 `StringBuilder` 序列,正好對應 VM 既有的
列印路徑 (與 Foo1 相同)。重新產生:

```bash
docker run --rm -v "$PWD":/w -w /w eclipse-temurin:8-jdk javac GEMM.java
```

**2. 為什麼邏輯放在建構子**

`simple_jvm_main.c` 只尋找並執行名為 `<init>` 的方法 (見 `findMethodInPool(... "<init>" ...)`),
所以範例把運算寫在建構子本體 (而非 `static main`),javac 會把它編進 `<init>`。

**3. 陣列模型 (handle-based heap)**

VM 原本只有 operand stack + `int locals[10]`,沒有物件/陣列概念。新增一個極簡 int 陣列 heap
在 `JvmContext` 裡:

```c
typedef struct { int *data; int length; } JvmArray;
// JvmContext: JvmArray arrays[256]; int array_count;
```

- `newarray` → `calloc` 一塊 int 陣列,把它在 `arrays[]` 的索引 (**handle**) 當作「陣列參考」push 上 stack。
- `astore_N` / `aload_N` → 把 handle 存進 / 取出區域變數 (與 int 共用 `locals.integer[]`)。
- `iaload` (`..., ref, index → value`) / `iastore` (`..., ref, index, value →`) → 用 handle 找到陣列再存取,**含邊界檢查**。

也就是說「陣列參考」在這台 VM 裡其實是一個小整數 handle,而非真正的指標 — 對教學夠用且安全。

**4. 迴圈 = 分支 opcode**

`for` 迴圈被 javac 編成計數器 + 條件分支。VM 新增三個 opcode 完成它:

- `iinc idx, const` → 區域變數原地遞增 (迴圈計數 `i++`)。
- `if_icmpge target` → pop 兩個 int,`value1 >= value2` 就跳到 target,否則往下 (`for` 的離開條件)。
- `goto target` → 無條件跳回迴圈頭。

分支怎麼跳: op 收到的 `opCode` 指向**目前指令的起點**,bytecode 裡的 offset 是相對位移,
所以 `*opCode += (signed16)offset` 就落在目標指令。`executeMethod` 的主迴圈每次重讀 `pc[0]`
分派,分支只是改 `*opCode`,不需要額外機制。

**5. 列印沿用既有路徑**

`System.out.println("c[0] = " + c[0])` 走的是 `getstatic` (System.out) → `new`/`dup`/`invokespecial`
(StringBuilder) → `invokevirtual append` → `invokevirtual println`,與 Foo1 完全相同的序列,
不需新增 opcode。差別只在 append 的 int 來自 `iaload` (陣列載入) 而非 `iload`。

### 為了 GEMM 新增的 opcode

| opcode | 0x | 作用 |
|---|---|---|
| `newarray` | BC | 配置 int 陣列,push handle |
| `aload` / `aload_2` / `aload_3` | 19 / 2C / 2D | 載入陣列參考 (handle) |
| `astore` / `astore_2` / `astore_3` | 3A / 4D / 4E | 儲存陣列參考 |
| `iaload` / `iastore` | 2E / 4F | 陣列元素讀 / 寫 (含邊界檢查) |
| `iinc` | 84 | 區域變數原地遞增 |
| `if_icmpge` / `goto` | A2 / A7 | 條件 / 無條件分支 (迴圈) |

`GEMM.class` 的輸出已納入 golden 回歸 (`test/golden/jvm_GEMM.txt`),`./build.sh test` 會驗證。

## 已知限制 (教學取捨)

- 以整數運算為主,型別系統不完整。
- 固定大小的 pool / register bank (例: `regs[32]`、`utf8CP[200]`),未做動態擴張。
- 僅實作範例程式 (Foo1 / GEMM) 用到的 bytecode 與 `java.lang.*` 子集。
- 陣列只支援 **一維 int 陣列** (handle-based,無真正物件模型 / GC / 多維 / 物件陣列)。
- 字串串接需用 JDK 8 編譯 (依賴 `StringBuilder` 序列,不支援 JDK 9+ 的 `invokedynamic`)。
