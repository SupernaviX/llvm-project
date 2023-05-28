#include "MCTargetDesc/V810InstPrinter.h"
#include "V810.h"
#include "V810TargetMachine.h"
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

    void emitInstruction(const MachineInstr *MI) override;

    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         const char *ExtraCode, raw_ostream &O) override;
  };
} // end of anonymous namespace

void V810AsmPrinter::emitInstruction(const MachineInstr *MI) {
  V810_MC::verifyInstructionPredicates(MI->getOpcode(),
                                       getSubtargetInfo().getFeatureBits());

  MCInst TmpInst;
  LowerV810MachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool V810AsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     const char *ExtraCode,
                                     raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1] != 0) return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default:
      // See if this is a generic print operand
      return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);
    case 'r':
     break;
    }
  }

  const MachineOperand &MO = MI->getOperand(OpNo);
  assert(MO.getType() == MachineOperand::MO_Register);
  O << StringRef(V810InstPrinter::getRegisterName(MO.getReg()));
  return false;
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810AsmPrinter() {
  RegisterAsmPrinter<V810AsmPrinter> X(getTheV810Target());
}