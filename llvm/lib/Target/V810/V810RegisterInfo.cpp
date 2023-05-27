#include "V810RegisterInfo.h"
#include "V810FrameLowering.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "V810GenRegisterInfo.inc"

V810RegisterInfo::V810RegisterInfo() : V810GenRegisterInfo(V810::R31) {}

const MCPhysReg*
V810RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

const uint32_t *
V810RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  return CSR_RegMask;
}

BitVector
V810RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(V810::R0); // zero register
  Reserved.set(V810::R1); // useful for ASM things
  Reserved.set(V810::R3); // stack pointer
  Reserved.set(V810::R4); // global pointer
  Reserved.set(V810::R10); // return value
  Reserved.set(V810::R30); // mul/div overflow
  Reserved.set(V810::R31); // return address

  Reserved.set(V810::SR5); // PSW should always be defined
  return Reserved;
}

bool
V810RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  MachineFunction &MF = *MI.getParent()->getParent();
  const V810FrameLowering *TFI = getFrameLowering(MF);

  Register FrameReg;
  int Offset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg).getFixed();
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
  MI.getOperand(FIOperandNum + 1).setImm(Offset);

  return false;
}

Register
V810RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return V810::R3;
}



