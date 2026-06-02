# Simple JVM & Dalvik VM — 用 C 看懂虛擬機怎麼跑 bytecode

這是一個**教學用**的精簡 Java 虛擬機 (JVM) 與 Android Dalvik 虛擬機 (DVM),以 C 撰寫。
目標只有一個:**讓初學者真正看懂「一個 `.class` / `.dex` 檔是怎麼被讀進來、然後一行一行
執行的」**。它刻意「夠用就好」(以整數運算為主),不是完整、不追求效能,但每個環節都看得到、改得動。

> 原作者: Chun-Yu Wang (wicanr2@gmail.com), 2013。
> 本版加上跨平台 build、確定性回歸測試、repo 整理,以及一個 **GEMM (矩陣乘法) 範例** —
> 同一個演算法分別在 JVM 與 DVM 上跑,用來對照兩種虛擬機最根本的差異。

---

## 1. 先建立直覺:什麼是虛擬機 / bytecode?

你寫的 `Foo.java` 不會直接被 CPU 執行。流程是這樣:

```
   Foo.java  ──(javac 編譯)──▶  Foo.class   ──(JVM 執行)──▶  輸出
  (人看的原始碼)              (一串 bytecode)            (一行一行解釋執行)
```

- **bytecode**:一種「給虛擬機看的中間語言」,比機器碼好懂、又跟 CPU 無關 → 所以 Java 能
  「一次編譯,到處執行」。每個指令叫一個 **opcode** (例如 `iadd` = 整數相加)。
- **虛擬機 (VM)**:一支程式,負責把 bytecode 一個一個讀出來解釋執行。本專案就是用 C 寫了
  兩支這樣的程式。

Android 不用標準 JVM,而是用 **Dalvik VM**,吃的是 `.dex` 檔。`.dex` 由 `.class` 再轉一手:

```
  Foo.java ─javac─▶ Foo.class ─dx─▶ Foo.dex ─(Dalvik VM 執行)─▶ 輸出
```

---

## 2. 最關鍵的一課:Stack-based (JVM) vs Register-based (Dalvik)

兩種 VM 最根本的差別,是**運算元 (operand) 放在哪裡**:

| | **JVM (堆疊機)** | **Dalvik (暫存器機)** |
|---|---|---|
| 運算元放哪 | 一個 **operand stack**,push/pop | 一組 **虛擬暫存器** v0, v1, v2 … |
| 指令長相 | 不帶位置,隱含對堆疊頂端操作 | 直接指名暫存器 |
| `c = a + b` | `iload a; iload b; iadd; istore c` | `add-int vc, va, vb` |
| 指令數 | 多 (一堆 push/pop) | 少 (一條搞定) |
| 指令大小 | 小 (1 byte 居多) | 大 (常 2~6 bytes) |

**用 GEMM 內層的 `sum += a[i*n+k] * b[k*n+j]` 親眼對照** (本專案兩個範例編出來的真實 bytecode):

```
── JVM (stack-based,什麼都先丟上堆疊) ──        ── Dalvik (register-based,直接指名暫存器) ──
  iload  7        // 把 sum 推上堆疊                mul-int      v8, v4, v10   // v8 = i*n
  aload_2         // 把陣列 a 推上堆疊              add-int/2addr v8, v0       // v8 = v8 + k
  iload 5; iload_1; imul; iload 8; iadd            aget         v8, v5, v8    // v8 = a[v8]
                  //   堆疊上算出 i*n+k             mul-int      v9, v0, v10   // v9 = k*n
  iaload          // a[i*n+k] (取代堆疊頂兩項)      add-int/2addr v9, v3       // v9 = v9 + j
  aload_3; iload 8; iload_1; imul; iload 6; iadd   aget         v9, v6, v9    // v9 = b[v9]
  iaload          // b[k*n+j]                       mul-int/2addr v8, v9       // v8 = v8 * v9
  imul            // a[..] * b[..]                  add-int/2addr v2, v8       // sum(v2) += v8
  iadd            // sum + ...
  istore 7        // 存回 sum
```

看出來了嗎?**同一行 Java,JVM 用「把東西搬上搬下堆疊」表達,Dalvik 用「對暫存器直接動手」
表達**。這就是兩種 VM 設計哲學的核心,也是 Android 當年選 register-based 的理由之一
(指令數少、解譯迴圈跑得快,適合早期手機)。

本專案的兩支 VM,程式結構也忠實反映這個差別:
- `simple_jvm` 裡有 `StackFrame` (operand stack) + `pushInt/popInt`。
- `simple_dvm` 裡有 `regs[32]` (暫存器組) + `load_reg_to/store_to_reg`。

---

## 3. 這支 toy VM 內部怎麼運作?

不管 JVM 或 DVM,骨架都是三步,對照原始碼很好讀:

1. **Parse (解析檔案)** — 把 `.class` / `.dex` 的二進位結構讀成 C struct。
   每個區段一個 parser 檔,例如 `simple_jvm_constant_pool_parser.c`、`simple_dvm_string_ids_parser.c`。
2. **Dispatch table (查表分派)** — 一張 `{ 名稱, opcode, 長度, 處理函式 }` 的表 (`byteCodes[]`)。
   讀到一個 opcode byte,就查表找到對應的 C 函式。
3. **Execute loop (執行迴圈)** — 從進入點方法的 code 開始,讀一個 opcode → 執行 → 推進 `pc`
   (program counter,指向下一條指令),直到方法結束。**分支 (迴圈/if) 就是改 `pc`**。

```c
// 執行迴圈的精神 (簡化自 executeMethod / runMethod)
while (還有指令) {
    opcode = code[pc];
    func   = 查表(opcode);     // dispatch table
    func(...);                 // 執行,函式內部會更新 pc (一般 +指令長度;分支則跳)
}
```

> 入口慣例:本 toy **JVM 執行 `<init>` (建構子)**,本 toy **DVM 執行 `main`**。這是這兩支
> 教學程式各自的簡化選擇,不是 Java 規範 — 範例程式因此把邏輯放在不同方法 (見下)。

---

## 4. GEMM 範例 — 同一個矩陣乘法,兩台 VM 都跑得動

`GEMM.java` (JVM 版) / `GEMMDvm.java` (DVM 版) 計算 **C = A × B** 的 2×2 整數矩陣乘法,
用來示範 VM 如何執行 **陣列 + 巢狀迴圈** 的 bytecode。兩者算出同一個答案:

```
A = [[1,2],[3,4]]   B = [[5,6],[7,8]]   ⇒   C = [[19,22],[43,50]]
```

```java
// 演算法本體 (兩版相同;JVM 版放在建構子 <init>,DVM 版放在 main)
int n = 2;
int[] a = {1,2,3,4};   // row-major: a[i*n+j]
int[] b = {5,6,7,8};
int[] c = new int[4];
for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
        int sum = 0;
        for (int k = 0; k < n; k++)
            sum += a[i*n+k] * b[k*n+j];   // 內積
        c[i*n+j] = sum;
    }
// 之後用 System.out.println 印出 c[0..3]
```

執行:

```bash
build/simple_jvm GEMM.class      # JVM (stack-based) 版
build/simple_dvm GEMMDvm.dex     # Dalvik (register-based) 版
# 兩者都輸出 c[0]=19  c[1]=22  c[2]=43  c[3]=50
```

### 為了跑 GEMM,兩台 VM 各補了哪些 opcode

原本兩台 VM 只夠跑「直線型」的算術 + 列印。GEMM 需要**陣列**和**迴圈 (= 分支)**,所以各補上:

| 能力 | JVM 新增 (stack) | Dalvik 新增 (register) |
|---|---|---|
| 配置陣列 | `newarray` | `new-array` |
| 讀 / 寫陣列元素 | `iaload` / `iastore` | `aget` / `aput` |
| 載入 / 存放陣列參考 | `aload(_2/_3)` / `astore(_2/_3)` | `move` |
| 迴圈計數 | `iinc` | `add-int/lit8` |
| 條件 / 無條件跳轉 | `if_icmpge` / `goto` | `if-ge` / `goto` |

> **陣列怎麼做的**:兩台 VM 都沒有真正的物件模型,所以用一個極簡的 **int 陣列 heap** —
> `newarray` 配一塊記憶體,回傳它的索引 (**handle**) 當作「陣列參考」,在堆疊 / 暫存器裡流動;
> `iaload`/`aget` 等再用 handle 找到陣列存取 (含邊界檢查)。對教學夠用且安全,但沒有 GC、
> 不支援多維 / 物件陣列。
>
> **分支怎麼做的**:opcode 收到目前指令的位置,bytecode 裡的 offset 是相對位移,把它加到
> `pc` 就跳到目標。Dalvik 的 offset 以 **16-bit code unit** 計 (要 ×2 換成 byte),JVM 以 byte 計。

### 浮點版 GEMM — 真正算小數 (double)

`GEMMf.java` (JVM) / `GEMMfDvm.java` (DVM) 把矩陣換成 `double`,算真正的浮點數:

```
A = [[1.1,2.2],[3.3,4.4]]   B = [[0.5,1.5],[2.5,3.5]]   ⇒   C = [[6.05,9.35],[12.65,20.35]]
```

```bash
build/simple_jvm GEMMf.class       # c[0]=6.05 c[1]=9.35 c[2]=12.65 c[3]=20.35
build/simple_dvm GEMMfDvm.dex      # 同上
```

**這裡又看到 stack vs register 的差別,而且更明顯** — 因為 `double` 是 **64 位元**:

| | JVM (堆疊機) | Dalvik (暫存器機) |
|---|---|---|
| 一個 double 佔 | operand stack 上 **2 個 slot** | **一對相鄰暫存器** (v, v+1) |
| double 陣列讀寫 | `daload` / `dastore` | `aget-wide` / `aput-wide` |
| double 區域變數 | `dload` / `dstore` (跨兩 slot) | (用暫存器對,本就成對) |
| 載入 double 常數 | `dconst_0`、`ldc2_w` (從常數池) | `const-wide`、`const-wide/16` |
| double 加 / 乘 | `dadd` / `dmul` | `add-double/2addr` / `mul-double/2addr` |

> **實作小故事 (踩到的坑)**:JVM 的 `ldc2_w` 推上堆疊的其實是「常數池索引」而非 double 值本身,
> 所以 `dastore` (存進陣列) 必須先用 `get_double_parameter` 把索引換回真正的 double — 否則初始化
> `a[0]=1.1` 會存進垃圾,矩陣全變 0。Dalvik 端則是 64 位元值要正確地拆/拼到「暫存器對」,本專案
> 直接沿用既有 `add-double/2addr` 的存取慣例以確保一致。
>
> 列印 double:`%g` 格式 (例 `9.35`);真實 JVM 因全精度會印 `9.350000000000001`,數值相同、
> 只是顯示精簡。

為它新增的 opcode:

| 能力 | JVM 新增 | Dalvik 新增 |
|---|---|---|
| double 陣列讀 / 寫 | `daload` / `dastore` | `aget-wide` / `aput-wide` |
| double 區域變數 | `dload` / `dstore` | (暫存器對) |
| double 常數 / 0.0 | `dconst_0` | `const-wide` / `const-wide/16` |
| `newarray` 支援 `double[]` | (擴充 atype) | (偵測 `[D` 型別) |

### 更多運算 — Ops 範例 (取模 / 位元 / 比較 / 負數)

`Ops.java` (JVM) / `OpsDvm.java` (DVM) 再示範一批常見運算與控制流,進一步擴大 opcode 覆蓋:
歐幾里得 GCD (取模 + while)、階乘 (for 迴圈)、位元 AND/OR/XOR、min/max (三元/比較分支)、負數。

```bash
build/simple_jvm Ops.class       # JVM
build/simple_dvm OpsDvm.dex      # DVM
# gcd(48,36)=12  6!=720  240&60=48  240|60=252  240^60=204  max=12  min=7  -7=-7
```

為它新增的 opcode:

| 能力 | JVM 新增 | Dalvik 新增 |
|---|---|---|
| 位元 AND/OR/XOR | `iand` / `ior` / `ixor` | (被 dx 常數摺疊,無需新增) |
| 取負 | `ineg` | `neg-int` |
| 取模 | (已有 `irem`) | `rem-int/2addr` |
| 與 0 比較分支 | `ifeq` | `if-eqz` |
| 大於 / 小於等於分支 | `if_icmpgt` / `if_icmple` | `if-gt` |

> **有趣的對照**:同一份 Java,**JVM 端 javac 保留了 `iand`/`ior`/`ixor`,但 DVM 端的 `dx`
> 把 `240 & 60` 這種「常數運算」直接算成結果塞進 `const`** (constant folding) — 所以 Dalvik 版
> 反而少補幾個 opcode。位移 `5 << 3` 兩邊都被摺疊掉了。這說明不同工具鏈的最佳化程度不同。

### 重新產生 class / dex (需 docker,符合本專案 docker-first 原則)

字串串接 (`"c = " + x`) 必須用 **JDK 8** 編譯:JDK 9+ 會編成 `invokedynamic`,本 VM 不支援;
JDK 8 編成 `StringBuilder` 序列,正好對應 VM 既有的列印路徑。`.dex` 還要再用 `dx` (見 `lib/`)
從 `.class` 轉,而 `dx 2.0.2` 只吃到 Java 6 (`-target 1.6`):

```bash
# JVM: GEMM.java -> GEMM.class
docker run --rm -v "$PWD":/w -w /w eclipse-temurin:8-jdk javac GEMM.java
# DVM: GEMMDvm.java -> GEMMDvm.class -> GEMMDvm.dex
docker run --rm -v "$PWD":/w -v "$PWD/lib":/jars -w /w eclipse-temurin:8-jdk bash -c \
  'javac -source 1.6 -target 1.6 GEMMDvm.java && java -jar /jars/dx.jar --dex --output=GEMMDvm.dex GEMMDvm.class'
```

---

## 5. 專案結構

```
simple_jvm/    JVM: 解析 .class → 跑 <init> 的 bytecode (stack-based)
simple_dvm/    DVM: 解析 .dex   → 跑 main 的 bytecode   (register-based)
test/          golden 回歸測試 (run_golden.sh / run_e2e.sh + golden/)
Foo1.java      基本範例 (算術 + 列印);Foo1.class / Foo1.dex 為 fixture
GEMM.java      JVM 版矩陣乘法 (陣列+迴圈);GEMM.class 為 fixture
GEMMDvm.java   DVM 版矩陣乘法;GEMMDvm.dex 為 fixture
GEMMf.java     JVM 版浮點矩陣乘法 (double, 真小數);GEMMf.class 為 fixture
GEMMfDvm.java  DVM 版浮點矩陣乘法;GEMMfDvm.dex 為 fixture
Ops.java       JVM 版運算示範 (取模/位元/比較/負數);Ops.class 為 fixture
OpsDvm.java    DVM 版運算示範;OpsDvm.dex 為 fixture
examples/      與 VM 無關的小範例 (c_cpp_linkage)
lib/           smali / baksmali / dx (產生 / 反組譯 dex 的工具)
Makefile / Dockerfile / build.sh   docker-first build
CONTEXT.md     術語表 (ubiquitous language)
docs/          重構報告
```

每個 parser 依 concern 切一檔,方便逐塊閱讀。術語見 `CONTEXT.md`。

## 6. Build (docker-first)

```bash
./build.sh          # build simple_jvm + simple_dvm → build/
./build.sh debug    # 帶 -DSIMPLE_*_DEBUG 的 debug 版 (印出每條指令的細節)
./build.sh clean
```

`build.sh` 以目前使用者身分在 `gcc:13` 容器內跑 `make`,不污染系統。若本機已有 gcc/make,
也可直接 `make`。

## 7. 執行

```bash
build/simple_jvm Foo1.class
build/simple_jvm GEMM.class
build/simple_dvm Foo1.dex
build/simple_dvm GEMMDvm.dex
build/simple_dvm Foo1.dex 4      # 第二個參數 = verbose level,印出每條指令的執行細節 (很適合學習!)
```

> 想看 VM「一條一條怎麼跑」,用 debug build 或 DVM 的 verbose level — 會印出每個 opcode 對
> 堆疊 / 暫存器做了什麼,是理解 bytecode 最直接的方式。

## 8. 測試

```bash
./build.sh test     # build 後跑 golden 回歸 (6 個案例: Foo1/GEMM × JVM/DVM + debug)
./build.sh e2e      # 完整 e2e (含 JVM==DVM 結果一致性檢查)
```

golden 測試以固定亂數種子 `SVM_SEED` 跑,比對輸出與 `test/golden/` 的已知良好結果。**刻意**
改動行為後更新基準:`UPDATE=1 ./test/run_golden.sh`。

### 為什麼需要 SVM_SEED

`Math.random` 用 `srand(time(0))`,輸出每次不同,無法回歸測試。設 `SVM_SEED=<n>` 可固定種子
讓輸出可重現;未設時維持原 `time(0)` 行為。

## 9. 已知限制 (教學取捨)

- 以整數運算為主,型別系統不完整。
- 固定大小的 pool / register bank (例: `regs[32]`、`utf8CP[200]`),未做動態擴張。
- 僅實作範例 (Foo1 / GEMM) 用到的 bytecode 與 `java.lang.*` 子集。
- 陣列只支援 **一維 int / double 陣列** (handle-based,無物件模型 / GC / 多維 / 物件陣列)。
- 字串串接需 JDK 8 編譯;`.dex` 需 `dx` 以 `-target 1.6` 產生。
- DVM 的方法查找是為範例結構簡化的 (`class_idx = i-1`),換複雜程式可能要調整。

## 10. 名詞速查

- **opcode** — 一條 bytecode 指令的數值碼 (如 `iadd`=0x60)。
- **operand stack** — JVM 放運算元的堆疊 (stack-based 的核心)。
- **register (vN)** — Dalvik 放運算元的虛擬暫存器 (register-based 的核心)。
- **constant pool** — class 檔裡的常數表 (字串、數字、方法/欄位參考)。
- **pc (program counter)** — 指向「下一條要執行的指令」的位置;分支 = 改 pc。
- **handle** — 本 VM 用來代表「陣列參考」的小整數 (陣列在 heap 表中的索引)。

延伸閱讀:JVM 規範的 instruction set、Dalvik 的 `dalvik-bytecode` 文件,以及 `CONTEXT.md`。
