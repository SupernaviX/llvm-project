#ifndef LLVM_LIB_TARGET_V810_V810_H
#define LLVM_LIB_TARGET_V810_V810_H

#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class FunctionPass;
class PassRegistry;
class V810TargetMachine;

FunctionPass *createV810IselDag(V810TargetMachine &TM);

void initializeV810DAGToDAGISelPass(PassRegistry &);
}

#endif