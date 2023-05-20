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
    ICC_BV = 0, // Overflow
    ICC_BC = 1, // Carry/Lower (unsigned)
    ICC_BE = 2, // Equal/Zero
    ICC_BNH = 3, // Not higher (unsigned)
    ICC_BN = 4, // Negative
    ICC_BR = 5, // Always (unconditional branch)
    ICC_BLT = 6, // Less than (signed)
    ICC_BLE = 7, // Less than or equal (signed)
    ICC_BNV = 8, // Not overflow
    ICC_BNC = 9, // Not carry/lower (unsigned)
    ICC_BNE = 10, // Not equal/zero
    ICC_BH = 11, // Nigher (unsigned)
    ICC_BP = 12, // Positive
    ICC_NOP = 13, // Never (nop)
    ICC_BGE = 14, // Greater than or equal (signed)
    ICC_BGT = 15, // Greater than
  };
  }

  inline static const char *V810CondCodeToString(V810CC::CondCodes CC) {
    switch (CC) {
    case V810CC::ICC_BV:  return "v";
    case V810CC::ICC_BC:  return "c";
    case V810CC::ICC_BE:  return "e";
    case V810CC::ICC_BNH: return "nh";
    case V810CC::ICC_BN:  return "n";
    case V810CC::ICC_BR:  return "r";
    case V810CC::ICC_BLT: return "lt";
    case V810CC::ICC_BLE: return "le";
    case V810CC::ICC_BNV: return "nv";
    case V810CC::ICC_BNC: return "nc";
    case V810CC::ICC_BNE: return "ne";
    case V810CC::ICC_BH:  return "h";
    case V810CC::ICC_BP:  return "p";
    case V810CC::ICC_NOP: return "nop";
    case V810CC::ICC_BGE: return "ge";
    case V810CC::ICC_BGT: return "gt";
    }
    llvm_unreachable("Invalid cond code");
  }
}

#endif