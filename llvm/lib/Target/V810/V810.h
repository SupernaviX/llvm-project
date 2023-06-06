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

namespace llvm {
  // Keep these in sync with V810InstrInfo.td
  namespace V810CC {
  enum CondCodes {
    CC_V = 0, // Overflow
    CC_C = 1, // Carry/Lower (unsigned)
    CC_E = 2, // Equal/Zero
    CC_NH = 3, // Not higher (unsigned)
    CC_N = 4, // Negative
    CC_BR = 5, // Always (unconditional branch)
    CC_LT = 6, // Less than (signed)
    CC_LE = 7, // Less than or equal (signed)
    CC_NV = 8, // Not overflow
    CC_NC = 9, // Not carry/lower (unsigned)
    CC_NE = 10, // Not equal/zero
    CC_H = 11, // Higher (unsigned)
    CC_P = 12, // Positive
    CC_NOP = 13, // Never (nop)
    CC_GE = 14, // Greater than or equal (signed)
    CC_GT = 15, // Greater than
  };
  }

  inline static const char *V810CondCodeToString(V810CC::CondCodes CC) {
    switch (CC) {
    case V810CC::CC_V:  return "v";
    case V810CC::CC_C:  return "c";
    case V810CC::CC_E:  return "e";
    case V810CC::CC_NH: return "nh";
    case V810CC::CC_N:  return "n";
    case V810CC::CC_BR:  return "r";
    case V810CC::CC_LT: return "lt";
    case V810CC::CC_LE: return "le";
    case V810CC::CC_NV: return "nv";
    case V810CC::CC_NC: return "nc";
    case V810CC::CC_NE: return "ne";
    case V810CC::CC_H:  return "h";
    case V810CC::CC_P:  return "p";
    case V810CC::CC_NOP: return "nop";
    case V810CC::CC_GE: return "ge";
    case V810CC::CC_GT: return "gt";
    }
    llvm_unreachable("Invalid cond code");
  }
}

#endif