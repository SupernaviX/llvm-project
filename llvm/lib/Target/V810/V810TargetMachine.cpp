#include "TargetInfo/V810TargetInfo.h"
#include "V810.h"
#include "V810MachineFunctionInfo.h"
#include "V810TargetMachine.h"
#include "V810TargetObjectFile.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810Target() {
  // Register the target.
  RegisterTargetMachine<V810TargetMachine> X(getTheV810Target());
}

static std::string computeDataLayout(const Triple &T) {
  return "e-p:32:16-i32:32";
}

V810TargetMachine::V810TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(
        T, computeDataLayout(TT), TT, CPU, FS, Options,
        RM.value_or(Reloc::Static),
        CM.value_or(CodeModel::Small),
        OL),
      TLOF(std::make_unique<V810TargetObjectFile>()) {
  initAsmInfo();
}

const V810Subtarget *
V810TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;
  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<V810Subtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
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
  void addPreEmitPass2() override;
};
} // namespace

TargetPassConfig *V810TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new V810PassConfig(*this, PM);
}

bool V810PassConfig::addInstSelector() {
  addPass(createV810IselDag(getV810TargetMachine()));
  return false;
}

void V810PassConfig::addPreEmitPass2() {
  addPass(createV810BranchSelectionPass());
}

V810TargetMachine::~V810TargetMachine() {}