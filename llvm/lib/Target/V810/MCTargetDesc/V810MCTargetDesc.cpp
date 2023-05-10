#include "V810MCTargetDesc.h"
#include "V810MCAsmInfo.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "V810GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "V810GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "V810GenRegisterInfo.inc"

static MCAsmInfo *createV810MCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new V810AsmInfo(TT);
  return MAI;
}

static MCInstrInfo *createV810MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitV810MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createV810MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitV810MCRegisterInfo(X, V810::R31);
  return X;
}

static MCSubtargetInfo *
createV810MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createV810MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}


extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810TargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheV810Target(), createV810MCAsmInfo);

  Target *T = &getTheV810Target();

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(*T, createV810MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(*T, createV810MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(*T, createV810MCSubtargetInfo);
}