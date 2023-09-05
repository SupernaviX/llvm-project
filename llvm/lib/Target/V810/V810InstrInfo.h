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
  virtual void anchor();
  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  unsigned isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  unsigned isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

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

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc, int64_t Offset) const override;

  bool isUnpredicatedTerminatorBesidesNop(const MachineInstr &MI) const;

public:
  explicit V810InstrInfo();
  const V810RegisterInfo &getRegisterInfo() const { return RI; }

  virtual unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  virtual ScheduleHazardRecognizer *
  CreateTargetPostRAHazardRecognizer(const MachineFunction &MF) const override;
  virtual ScheduleHazardRecognizer *
  CreateTargetPostRAHazardRecognizer(const InstrItineraryData *II,
                                     const ScheduleDAG *DAG) const override;
  virtual ScheduleHazardRecognizer *
  CreateTargetMIHazardRecognizer(const InstrItineraryData *II,
                                 const ScheduleDAGMI *DAG) const override;
};

}

#endif