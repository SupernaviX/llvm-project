#ifndef LLVM_LIB_TARGET_V810_V810_H
#define LLVM_LIB_TARGET_V810_V810_H

#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class AsmPrinter;
class FunctionPass;
class MachineInstr;
class MCInst;
class PassRegistry;
class V810TargetMachine;

FunctionPass *createV810IselDag(V810TargetMachine &TM);

void LowerV810MachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                   AsmPrinter &AP);

void initializeV810DAGToDAGISelPass(PassRegistry &);
}

#endif