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
    ICC_V = 0, // Overflow
    ICC_C = 1, // Carry/Lower (unsigned)
    ICC_E = 2, // Equal/Zero
    ICC_NH = 3, // Not higher (unsigned)
    ICC_N = 4, // Negative
    ICC_BR = 5, // Always (unconditional branch)
    ICC_LT = 6, // Less than (signed)
    ICC_LE = 7, // Less than or equal (signed)
    ICC_NV = 8, // Not overflow
    ICC_NC = 9, // Not carry/lower (unsigned)
    ICC_NE = 10, // Not equal/zero
    ICC_H = 11, // Higher (unsigned)
    ICC_P = 12, // Positive
    ICC_NOP = 13, // Never (nop)
    ICC_GE = 14, // Greater than or equal (signed)
    ICC_GT = 15, // Greater than
  };
  }

  inline static const char *V810CondCodeToString(V810CC::CondCodes CC) {
    switch (CC) {
    case V810CC::ICC_V:  return "v";
    case V810CC::ICC_C:  return "c";
    case V810CC::ICC_E:  return "e";
    case V810CC::ICC_NH: return "nh";
    case V810CC::ICC_N:  return "n";
    case V810CC::ICC_BR:  return "r";
    case V810CC::ICC_LT: return "lt";
    case V810CC::ICC_LE: return "le";
    case V810CC::ICC_NV: return "nv";
    case V810CC::ICC_NC: return "nc";
    case V810CC::ICC_NE: return "ne";
    case V810CC::ICC_H:  return "h";
    case V810CC::ICC_P:  return "p";
    case V810CC::ICC_NOP: return "nop";
    case V810CC::ICC_GE: return "ge";
    case V810CC::ICC_GT: return "gt";
    }
    llvm_unreachable("Invalid cond code");
  }
}

#endif