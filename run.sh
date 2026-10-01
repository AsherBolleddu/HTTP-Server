#!/bin/sh
set -e # Exit early if any commands fail

config=Debug
if [ "$1" = "--release" ]; then
  config=Release
  shift # drop --release so the server doesn't see it
fi

(
  cd "$(dirname "$0")"
  cmake -B build -S . -G "Ninja Multi-Config" -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  cmake --build ./build --config "$config"
)

exec "$(dirname "$0")/build/$config/http-server" "$@"
