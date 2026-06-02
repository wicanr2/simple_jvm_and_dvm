# CONTEXT — Ubiquitous Language

本 repo 的 domain glossary。命名變數 / 寫文件 / 討論時優先用這裡的詞。
格式: `Term — definition. _Avoid_: forbidden synonyms`。

## 通用

- **bytecode** — VM 執行的指令串。JVM 的 op 是 1 byte;Dalvik 以 16-bit 單元 (`ushort`) 編碼。
- **opcode** — 單一指令的數值碼。dispatch table (`byteCode[]`) 把 opcode 對到處理函式 `opCodeFunc`。
- **golden** — 已知良好的參考輸出,放在 `test/golden/`,回歸測試比對用。 _Avoid_: baseline、expected (在本 repo 統一稱 golden)。
- **SVM_SEED** — 固定亂數種子的環境變數,讓 `Math.random` 輸出可重現以利測試。

## JVM 側 (`simple_jvm/`)

- **ClassFileFormat** — 解析後的 `.class` 頂層結構 (magic / version / counts)。
- **constant pool** — class 檔的常數表;本實作按 tag 分桶 (`SimpleConstantPool` 內 utf8CP / integerCP / method …)。 _Avoid_: CP (寫全名)。
- **method pool / field pool / interface pool** — 對應 class 檔各區段的解析結果容器。
- **StackFrame** — operand stack;`pushInt`/`popDouble` 等操作其上的 `StackEntry`。 _Avoid_: operand stack 簡稱混用。
- **StackEntry** — stack 上單一槽,帶 `type` (INT/REF/LONG/DOUBLE/FLOAT)。
- **ref entry** — stack 上指向 constant pool index 的項;取值時需再到 pool 解參。

## Dalvik 側 (`simple_dvm/`)

- **DexFileFormat** — 解析後的 `.dex` 頂層結構,含 header 與各 ids 表。
- **DexHeader** — dex 檔頭 (magic / checksum / 各區段 size+off)。
- **string_ids / type_ids / proto_ids / field_ids / method_ids** — dex 的索引表;parser 各一檔。
- **class_def_item / class_data_item / encoded_method / code_item** — class 定義與方法碼結構。
- **map_list** — dex 各區段的索引地圖。
- **simple_dalvik_vm** — DVM 執行狀態:`heap`、`regs[32]` register bank、`result`、`pc`。 _Avoid_: 把 register bank 叫 stack (Dalvik 是 register-based)。
- **invoke_parameters** — `35c` 形式呼叫的參數打包 (method_id + reg 列表)。
- **uleb128** — dex 用的可變長度無號整數編碼。

## 型別別名 (注意)

- JVM: `u2` = `unsigned short`、`byte` = `unsigned char`。
- DVM: `u1/u2/u4` = unsigned 8/16/32-bit。`u2` 原為 `signed short`,已修正為 `unsigned short` (符合 DEX 規格)。

## Flagged ambiguities

- `byteCode` struct 與 `opCodeFunc` typedef 在 JVM/DVM 各有一份,**簽章不同** (JVM 傳
  `StackFrame*`;DVM 傳 `simple_dalvik_vm*`)。同名但非共用,勿誤合併。
