#ifndef LLVM_LIB_TARGET_V810_V810INSTRINFO_H
#define LLVM_LIB_TARGET_V810_V810INSTRINFO_H

#include "V810RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "V810GenInstrInfo.inc"

namespace llvm {

class V810InstrInfo : public V810GenInstrInfo {
  const V810RegisterInfo RI ;
  virtual void anchor();
  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, const DebugLoc &DL,
                           MCRegister DestReg, MCRegister SrcReg,
                           bool KillSrc) const override;
public:
  const V810RegisterInfo &getRegisterInfo() const { return RI; }
};

}

#endif