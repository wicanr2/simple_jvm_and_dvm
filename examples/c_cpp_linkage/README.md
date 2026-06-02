# C / C++ 連結 demo

與本專案的 JVM/DVM **無關**的小範例,示範 C (`main.c`) 透過 `extern "C"` 包裝
(`a.cpp` 的 `b_wrapper`) 呼叫 C++ namespace 內的函式 (`b.cpp` 的 `B::b_func`)。

原本散在 `test/` 目錄,因 `test/` 已改作 golden 回歸測試用途,移到此處保存。

```bash
g++ -c a.cpp b.cpp && gcc -c main.c && g++ main.o a.o b.o -o demo && ./demo
```
