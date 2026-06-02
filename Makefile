# Simple JVM / Dalvik VM — cross-platform build
#
# 用法 (建議在 docker 內執行，見 build.sh):
#   make            # build simple_jvm + simple_dvm (release)
#   make debug      # build 兩者的 debug 版 (帶 -DSIMPLE_*_DEBUG)
#   make test       # build 後跑 golden 回歸測試
#   make clean      # 移除 build/ 產物

CC      ?= gcc
CFLAGS  ?= -g -Wall -Wextra -Wno-unused-parameter
BUILD   := build

JVM_SRC := $(wildcard simple_jvm/simple_jvm*.c)
DVM_SRC := $(wildcard simple_dvm/simple_dvm*.c)

.PHONY: all jvm dvm debug test e2e clean

all: jvm dvm

jvm:   $(BUILD)/simple_jvm
dvm:   $(BUILD)/simple_dvm
debug: $(BUILD)/simple_jvm_debug $(BUILD)/simple_dvm_debug

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/simple_jvm: $(JVM_SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(JVM_SRC) -o $@

$(BUILD)/simple_jvm_debug: $(JVM_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -DSIMPLE_JVM_DEBUG $(JVM_SRC) -o $@

$(BUILD)/simple_dvm: $(DVM_SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(DVM_SRC) -o $@

$(BUILD)/simple_dvm_debug: $(DVM_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -DSIMPLE_DVM_DEBUG $(DVM_SRC) -o $@

test: all
	./test/run_golden.sh

e2e: all debug
	./test/run_e2e.sh

clean:
	rm -rf $(BUILD)
