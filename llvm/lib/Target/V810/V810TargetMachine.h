#ifndef LLVM_LIB_TARGET_V810_V810TARGETMACHINE_H
#define LLVM_LIB_TARGET_V810_V810TARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {

class V810TargetMachine : public LLVMTargetMachine {
public:
  V810TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOpt::Level OL,
                    bool JIT);
  ~V810TargetMachine() override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;
};

} // end namespace llvm

#endif
