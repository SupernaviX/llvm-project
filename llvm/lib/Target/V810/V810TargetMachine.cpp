#include "TargetInfo/V810TargetInfo.h"
#include "V810TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810Target() {
  // Register the target.
  RegisterTargetMachine<V810TargetMachine> X(getTheV810Target());
}

static std::string computeDataLayout(const Triple &T) {
  return "e-p:32:32-i32:32";
}

V810TargetMachine::V810TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(
        T, computeDataLayout(TT), TT, CPU, FS, Options,
        RM.value_or(Reloc::Static),
        CM.value_or(CodeModel::Small),
        OL
    ) {
}

V810TargetMachine::~V810TargetMachine() {}