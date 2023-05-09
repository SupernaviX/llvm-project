#ifndef LLVM_LIB_TARGET_V810_V810FRAMELOWERING_H
#define LLVM_LIB_TARGET_V810_V810FRAMELOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

class V810FrameLowering : public TargetFrameLowering {
public:
  explicit V810FrameLowering();

    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;
};

} // End llvm namespace

#endif