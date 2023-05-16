#ifndef LLVM_V810_FIXUP_KINDS_H
#define LLVM_V810_FIXUP_KINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace V810 {
// Keep this list in order with MCFixupKindInfo Infos in V810AsmBackend.cpp
enum Fixups {
  // A 26-bit PC-relative fixup.
  fixup_26_pcrel = FirstTargetFixupKind,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif