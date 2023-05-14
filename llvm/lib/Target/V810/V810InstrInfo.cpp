#include "V810InstrInfo.h"
#include "V810.h"
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
  BuildMI(MBB, I, DL, get(V810::MOVr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
}