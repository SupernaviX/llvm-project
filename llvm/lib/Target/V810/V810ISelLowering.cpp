#include "V810ISelLowering.h"

using namespace llvm;

V810TargetLowering::V810TargetLowering(const TargetMachine &TM,
                                       const V810Subtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  // TODO: this is where we start massaging LLVM concepts into V810 registers/instructions/etc
}