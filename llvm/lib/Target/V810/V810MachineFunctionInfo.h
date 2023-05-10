#ifndef LLVM_LIB_TARGET_V810_V810MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_V810_V810MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

  class V810MachineFunctionInfo : public MachineFunctionInfo {
    virtual void anchor();
  public:
    V810MachineFunctionInfo(const Function &f, const TargetSubtargetInfo *STI) {}
 
    MachineFunctionInfo *
    clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
          const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
        const override;
  };
}

#endif