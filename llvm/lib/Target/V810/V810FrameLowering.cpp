#include "V810FrameLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

V810FrameLowering::V810FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0, Align(4)) {}

void
V810FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  int bytes = (int) MF.getFrameInfo().getStackSize();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  moveStackPointer(MF, MBB, MBBI, bytes);
}

void
V810FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  int bytes = (int) MF.getFrameInfo().getStackSize();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  moveStackPointer(MF, MBB, MBBI, -bytes);
}

bool
V810FrameLowering::hasFP(const MachineFunction &MF) const {
  return false;
}

void
V810FrameLowering::moveStackPointer(MachineFunction &MF, MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI, int bytes) const {
  if (bytes == 0) {
    return;
  }

  DebugLoc dl;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  // TODO: use ADDri if offset is smol
  BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), V810::R3)
    .addReg(V810::R3).addImm(-bytes);
}
