#include "V810InstrInfo.h"
#include "V810.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "V810GenInstrInfo.inc"

void V810InstrInfo::anchor() {}

V810InstrInfo::V810InstrInfo()
  : V810GenInstrInfo(V810::ADJCALLSTACKDOWN, V810::ADJCALLSTACKUP), RI() {}

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

MachineBasicBlock *V810InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected opcode!");
  case V810::Bcond:
    return MI.getOperand(1).getMBB();
  case V810::JAL:
  case V810::JR:
    return MI.getOperand(0).getMBB();
  }
}

static bool isUncondBranch(MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default:        return false;
  case V810::JR:  return true;
  case V810::Bcond:
    return MI.getOperand(0).getImm() == V810CC::CC_BR;
  }
}

static bool isCondBranch(MachineInstr &MI) {
  return MI.getOpcode() == V810::Bcond
    && MI.getOperand(0).getImm() != V810CC::CC_BR
    && MI.getOperand(0).getImm() != V810CC::CC_NOP;
}

static bool isIndirectBranch(MachineInstr &MI) {
  return MI.getOpcode() == V810::JMP;
}

bool V810InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*I))
    return false;
  
  MachineInstr *LastInst = &*I;

  // function ends with only one terminator
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (isUncondBranch(*LastInst)) {
      TBB = getBranchDestBlock(*LastInst);
      return false;
    }
    if (isCondBranch(*LastInst)) {
      TBB = getBranchDestBlock(*LastInst);
      Cond.push_back(LastInst->getOperand(0));
      return false;
    }
    // can't handle indirect branches
    return true;
  }

  MachineInstr *SecondLastInst = &*I;

  // If we're allowed to, remove any unconditional branches after other unconditional branches
  if (AllowModify && isUncondBranch(*LastInst)) {
    while (isUncondBranch(*SecondLastInst)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
        // NOW it just ends with one unconditional branch
        TBB = getBranchDestBlock(*LastInst);
        return false;
      } else {
        SecondLastInst = &*I;
      }
    }
  }

  // Give up if there are more than three terminators
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;
  
  // conditional branch followed by unconditional branch
  if (isCondBranch(*SecondLastInst) && isUncondBranch(*LastInst)) {
    TBB = getBranchDestBlock(*SecondLastInst);
    FBB = getBranchDestBlock(*LastInst);
    Cond.push_back(SecondLastInst->getOperand(0));
    return false;
  }

  // indirect branch followed by anything, clear out the anything
  if (isIndirectBranch(*SecondLastInst) && isUncondBranch(*LastInst)) {
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
  }

  return true;
}

unsigned V810InstrInfo::insertBranch(MachineBasicBlock &MBB,
                                     MachineBasicBlock *TBB,
                                     MachineBasicBlock *FBB,
                                     ArrayRef<MachineOperand> Cond,
                                     const DebugLoc &DL,
                                     int *BytesAdded) const {
  assert(TBB);
  // NB: if these branches are too short, we will fix them later

  if (Cond.empty()) {
    // Unconditional branch
    assert(!FBB && "Unconditional branch with multiple successors");
    BuildMI(&MBB, DL, get(V810::JR)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded = 4;
    return 1;
  }

  // Conditional branch
  unsigned CC = Cond[0].getImm();
  BuildMI(&MBB, DL, get(V810::Bcond)).addImm(CC).addMBB(TBB);

  if (!FBB) {
    if (BytesAdded)
      *BytesAdded = 2;
    return 1;
  }

  BuildMI(&MBB, DL, get(V810::JR)).addMBB(FBB);
  if (BytesAdded)
    *BytesAdded = 6;
  return 2;
}

unsigned V810InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  int Removed = 0;
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;
    if (!isCondBranch(*I) && !isUncondBranch(*I))
      break; // Not a branch
    
    Removed += getInstSizeInBytes(*I);
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  if (BytesRemoved)
    *BytesRemoved = Removed;
  return Count;
}

bool V810InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.size() != 1) {
    llvm_unreachable(std::to_string(Cond.size()).c_str());
  }
  assert(Cond.size() == 1);
  V810CC::CondCodes CC = static_cast<V810CC::CondCodes>(Cond[0].getImm());
  Cond[0].setImm(InvertV810CondCode(CC));
  return false;
}

bool V810InstrInfo::isBranchOffsetInRange(unsigned BranchOpc, int64_t Offset) const {
  switch(BranchOpc) {
  default:
    llvm_unreachable("Unknown branch instruction!");
  case V810::Bcond:
    return isInt<9>(Offset);
  case V810::JR:
  case V810::JAL:
    return isInt<26>(Offset);
  }
}

unsigned V810InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.isInlineAsm()) {
    const MachineFunction *MF = MI.getParent()->getParent();
    const char *AsmStr = MI.getOperand(0).getSymbolName();
    return getInlineAsmLength(AsmStr, *MF->getTarget().getMCAsmInfo());
  }

  unsigned Opcode = MI.getOpcode();
  return get(Opcode).getSize();
}