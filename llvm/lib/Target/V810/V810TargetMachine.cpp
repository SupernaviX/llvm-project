#include "TargetInfo/V810TargetInfo.h"
#include "V810.h"
#include "V810MachineFunctionInfo.h"
#include "V810TargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
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
  initAsmInfo();
}

MachineFunctionInfo *V810TargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return V810MachineFunctionInfo::create<V810MachineFunctionInfo>(Allocator, F, STI);
}

namespace {
class V810PassConfig : public TargetPassConfig {
public:
  V810PassConfig(V810TargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  V810TargetMachine &getV810TargetMachine() const {
    return getTM<V810TargetMachine>();
  }

  bool addInstSelector() override;
};
} // namespace

bool V810PassConfig::addInstSelector() {
  // TODO: add the dang selector
  return false;
}

TargetPassConfig *V810TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new V810PassConfig(*this, PM);
}

V810TargetMachine::~V810TargetMachine() {}