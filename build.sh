#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

while getopts 'do:' opt; do
  case "$opt" in
    d)
      DEBUG=1
      ;;
    o)
      OUTPUT="$OPTARG"
      ;;
  esac
done

if [[ -z "$OUTPUT" ]]
then
    echo "-o <output-directory>"
    exit 1
fi

if [[ -n "$DEBUG" ]]
then
    BUILD_TYPE="Debug"
else
    BUILD_TYPE="Release"
fi

sudo apt update
sudo apt install -y cmake g++ makeself qt6-base-dev libqt6core6 libqt6keychain1 qtkeychain-qt6-dev libcurl4 libcurl4-openssl-dev

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -S $SCRIPT_DIR -B $OUTPUT
cd $OUTPUT
cmake --build . -j$(nproc)
