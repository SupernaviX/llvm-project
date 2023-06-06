#include "MCTargetDesc/V810MCTargetDesc.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "v810-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class V810Disassembler : public MCDisassembler {
public:
  V810Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~V810Disassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
}

static MCDisassembler *createV810Disassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new V810Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810Disassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheV810Target(),
                                         createV810Disassembler);
}

#include "V810GenDisassemblerTables.inc"

DecodeStatus V810Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  return DecodeStatus::Fail;
}