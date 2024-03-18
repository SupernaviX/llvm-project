// RUN: env SDKROOT="/" %clang -arch arm64 -c -### %s 2>&1 | \
// RUN:   FileCheck %s
//
// REQUIRES: system-darwin
// XFAIL: target=v810-{{.*}}
//
// CHECK: "-triple" "arm64-apple-macosx{{[0-9.]+}}"
