#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

namespace {
  class V810AsmBackend : public MCAsmBackend {
  private:
    Triple::OSType OSType;
  public:
    V810AsmBackend(Triple::OSType OSType) : MCAsmBackend(support::little), OSType(OSType) {}

    unsigned getNumFixupKinds() const override {
      return 0;
    }

    /// fixupNeedsRelaxation - Target specific predicate for whether a given
    /// fixup requires the associated instruction to be relaxed.
    bool fixupNeedsRelaxation(const MCFixup &Fixup,
                              uint64_t Value,
                              const MCRelaxableFragment *DF,
                              const MCAsmLayout &Layout) const override {
      // no fixups, no problems
      return false;
    }

    void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                    const MCValue &Target, MutableArrayRef<char> Data,
                    uint64_t Value, bool IsResolved,
                    const MCSubtargetInfo *STI) const override {
      // haven't implemented any
    }

    bool writeNopData(raw_ostream &OS, uint64_t Count,
                      const MCSubtargetInfo *STI) const override {
      // Cannot emit NOP with size not multiple of 16 bits.
      if (Count % 2 != 0) {
        return false;
      }

      uint64_t NumNops = Count / 2;
      for (uint64_t i = 0; i != NumNops; ++i) {
        // all 0s is MOV r0 r0, which takes one cycle and does nothing
        support::endian::write<uint16_t>(OS, 0x0000, support::little);
      }
      return true;
    }

    std::unique_ptr<MCObjectTargetWriter>
    createObjectTargetWriter() const override {
      uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
      return createV810ObjectWriter(OSABI);
    }
  };

} // end anonymous namespace

MCAsmBackend *llvm::createV810AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new V810AsmBackend(STI.getTargetTriple().getOS());
}