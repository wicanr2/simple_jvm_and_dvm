#!/usr/bin/env bash
#
# 在 docker 內執行 make，避免污染系統環境。
# 用法:
#   ./build.sh            # = make all
#   ./build.sh test       # 跑 golden 回歸測試
#   ./build.sh debug      # build debug 版
#   ./build.sh clean
#
set -euo pipefail

IMAGE=simple-vm-build
cd "$(dirname "$0")"

docker build -q -t "$IMAGE" -f Dockerfile . >/dev/null
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/src -w /src "$IMAGE" make "$@"
