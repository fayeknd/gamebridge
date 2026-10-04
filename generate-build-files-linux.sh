#!/bin/bash
cmake -S . -B build -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_C_FLAGS="-target x86_64-w64-windows-gnu -fuse-ld=lld" \
      -DCMAKE_CXX_FLAGS="-target x86_64-w64-windows-gnu -fuse-ld=lld" \
      -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++ -lwinpthread"