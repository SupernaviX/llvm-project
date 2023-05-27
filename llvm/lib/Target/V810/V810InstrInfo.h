#ifndef LLVM_LIB_TARGET_V810_V810INSTRINFO_H
#define LLVM_LIB_TARGET_V810_V810INSTRINFO_H

#include "V810RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "V810GenInstrInfo.inc"

namespace llvm {

class V810Subtarget;

class V810InstrInfo : public V810GenInstrInfo {
  const V810RegisterInfo RI;
  const V810Subtarget& Subtarget;
  virtual void anchor();
  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, const DebugLoc &DL,
                           MCRegister DestReg, MCRegister SrcReg,
                           bool KillSrc) const override;
  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator HI,
                                   Register SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI,
                                   Register VReg) const override;
  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI,
                                    Register DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI,
                                    Register VReg) const override;
public:
  explicit V810InstrInfo(V810Subtarget &ST);
  const V810RegisterInfo &getRegisterInfo() const { return RI; }
};

}

#endif