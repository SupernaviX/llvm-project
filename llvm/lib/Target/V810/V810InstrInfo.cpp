#include "V810InstrInfo.h"
#include "V810.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "V810GenInstrInfo.inc"

void V810InstrInfo::anchor() {}

void V810InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I, const DebugLoc &DL,
                                MCRegister DestReg, MCRegister SrcReg,
                                bool KillSrc) const {
  assert(V810::GenRegsRegClass.contains(DestReg, SrcReg));
  if (DestReg == SrcReg) {
    return;
  }
  BuildMI(MBB, I, DL, get(V810::MOVr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
}

void V810InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register SrcReg, bool isKill, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI,
                                        Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
    MachinePointerInfo::getFixedStack(*MF, FrameIndex), MachineMemOperand::MOStore,
    MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, I, DL, get(V810::ST_W)).addFrameIndex(FrameIndex).addImm(0)
    .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
}

void V810InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
    MachinePointerInfo::getFixedStack(*MF, FrameIndex), MachineMemOperand::MOLoad,
    MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));

  BuildMI(MBB, I, DL, get(V810::LD_W), DestReg).addFrameIndex(FrameIndex).addImm(0)
    .addMemOperand(MMO);
}
