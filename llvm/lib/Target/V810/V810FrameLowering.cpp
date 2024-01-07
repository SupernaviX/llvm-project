#include "V810.h"
#include "V810FrameLowering.h"
#include "V810RegisterInfo.h"
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

static bool isFPSave(MachineBasicBlock::iterator I) {
  return I->getOpcode() == V810::ST_W
    && I->getOperand(0).isFI()
    && I->getOperand(2).isReg()
    && I->getOperand(2).getReg() == V810::R2;
}

void
V810FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  int bytes = (int) MF.getFrameInfo().getStackSize();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  moveStackPointer(MF, MBB, MBBI, -bytes);
  if (hasFP(MF)) {
    // Find the instruction where we store FP...
    while (MBBI != MBB.getFirstTerminator() && !isFPSave(MBBI)) {
      ++MBBI;
      if (MBBI == MBB.getFirstTerminator()) {
        // If we don't store FP, don't worry about it.
        return;
      }
    }
    // Grab the frame index where we stored it...
    int FPIndex = MBBI->getOperand(0).getIndex();

    // and set FP to the address of that frame index.
    ++MBBI;
    DebugLoc dl;
    BuildMI(MBB, MBBI, dl, MF.getSubtarget().getInstrInfo()->get(V810::MOVEA), V810::R2)
      .addFrameIndex(FPIndex).addImm(0).setMIFlag(MachineInstr::FrameSetup);
  }

  assert(!MF.getSubtarget().getRegisterInfo()->hasStackRealignment(MF) && "Stack realignment not supported");
}

void
V810FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  int bytes = (int) MF.getFrameInfo().getStackSize();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  moveStackPointer(MF, MBB, MBBI, bytes);
}

MachineBasicBlock::iterator V810FrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = 0;
    switch (MI.getOpcode()) {
    default: llvm_unreachable("Unrecognized call frame pseudo");
    case V810::ADJCALLSTACKDOWN:
      Size = -MI.getOperand(0).getImm();
      break;
    case V810::ADJCALLSTACKUP:
      Size = MI.getOperand(0).getImm();
      break;
    }
    moveStackPointer(MF, MBB, I, Size);
  }
  return MBB.erase(I);
}

bool
V810FrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

void
V810FrameLowering::determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (hasFP(MF)) {
    // If we need to use FP, we always store the old value
    SavedRegs.set(V810::R2);
  }
}

void
V810FrameLowering::moveStackPointer(MachineFunction &MF, MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI, int bytes) const {
  if (bytes == 0) {
    return;
  }

  DebugLoc dl;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  if (isInt<5>(bytes)) {
    BuildMI(MBB, MBBI, dl, TII.get(V810::ADDri), V810::R3)
      .addReg(V810::R3).addImm(bytes);
  } else if (isInt<16>(bytes)) {
    BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), V810::R3)
      .addReg(V810::R3).addImm(bytes);
  } else {
    assert(isInt<32>(bytes));

    uint64_t lo = EvalLo(bytes);
    uint64_t hi = EvalHi(bytes);

    // We need a temporary register, but can't use virtual registers here.
    // R1 is reserved, but mainly meant to be used by crt0 for interrupt handling, so we steal that.
    // Turn off interrupts while we move this pointer, so that surprise interrupts don't break it.
    Register TempReg = V810::R1;
    // Force these to get scheduled in the right order with a fake implicit dependency.
    // That implicit dependency is the existing "sr29" system register because I am lazy.
    Register DepReg = V810::SR29;

    BuildMI(MBB, MBBI, dl, TII.get(V810::SEI)).addDef(DepReg, RegState::Implicit);
    BuildMI(MBB, MBBI, dl, TII.get(V810::MOVHI), TempReg)
      .addReg(V810::R0).addImm(hi).addUse(DepReg, RegState::Implicit);
    if (lo) {
      BuildMI(MBB, MBBI, dl, TII.get(V810::MOVEA), TempReg)
        .addReg(TempReg).addImm(lo);
    }
    BuildMI(MBB, MBBI, dl, TII.get(V810::ADDrr), V810::R3)
        .addReg(V810::R3).addReg(TempReg);
    BuildMI(MBB, MBBI, dl, TII.get(V810::CLI)).addUse(DepReg, RegState::Implicit);
  }
}
