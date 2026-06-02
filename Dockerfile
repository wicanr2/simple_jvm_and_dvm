# 編譯 / 測試環境 (符合 docker-first 規則)
# gcc:13 base 已含 make / build-essential。
FROM gcc:13
WORKDIR /src
