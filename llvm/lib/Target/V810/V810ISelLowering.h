#ifndef LLVM_LIB_TARGET_V810_V810ISELLOWERING_H
#define LLVM_LIB_TARGET_V810_V810ISELLOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class V810Subtarget;

  class V810TargetLowering : public TargetLowering {
  private:
    const V810Subtarget *Subtarget;
  public:
    V810TargetLowering(const TargetMachine &TM, const V810Subtarget &STI);
  };
}

#endif