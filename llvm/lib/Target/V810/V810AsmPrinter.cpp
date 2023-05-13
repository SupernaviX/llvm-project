#include "V810.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
  class V810AsmPrinter : public AsmPrinter {
  public:
    explicit V810AsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
        : AsmPrinter(TM, std::move(Streamer)) {}
    StringRef getPassName() const override { return "V810 Assembly Printer"; }
  };
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810AsmPrinter() {
  RegisterAsmPrinter<V810AsmPrinter> X(getTheV810Target());
}