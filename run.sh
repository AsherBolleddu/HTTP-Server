#!/bin/sh
set -e # Exit early if any commands fail

preset=debug
if [ "$1" = "--release" ]; then
  preset=release
  shift # drop --release so the server doesn't see it
fi

(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory
  cmake --preset default
  cmake --build --preset "$preset"
)

case "$preset" in
  debug)   config=Debug ;;
  release) config=Release ;;
esac

exec "$(dirname "$0")/build/$config/http-server" "$@"
