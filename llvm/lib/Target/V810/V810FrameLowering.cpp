#include "V810FrameLowering.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

V810FrameLowering::V810FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0, Align(4)) {}

void
V810FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {}

void
V810FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {}

bool
V810FrameLowering::hasFP(const MachineFunction &MF) const {
  return true;
}
