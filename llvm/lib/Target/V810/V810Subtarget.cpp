#include "V810Subtarget.h"
#include "V810.h"

using namespace llvm;

#define DEBUG_TYPE "v810-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "V810GenSubtargetInfo.inc"

V810Subtarget::V810Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : V810GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(), TLInfo(TM, *this), FrameLowering() {
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
}