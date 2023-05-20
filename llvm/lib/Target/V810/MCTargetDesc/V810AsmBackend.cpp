#include "MCTargetDesc/V810FixupKinds.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

// The v810 is little-endian, but stores 32-bit instructions with their high halfword first.
// We need to exchange halfwords after adjusting fixups to match this.
static uint64_t xh(uint64_t value) {
  return (value >> 16) | (value << 16);
}

static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {
  switch (Kind) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case V810::fixup_v810_lo:
    return xh(Value & 0x0000ffff);
  case V810::fixup_v810_hi:
    // if LO is negative, increment HI to compensate
    return xh((Value >> 16) & 0x0000ffff) + ((Value & 0x00008000) != 0);
  case V810::fixup_v810_9_pcrel:
    return Value & 0x01ff;
  case V810::fixup_v810_26_pcrel:
    return xh(Value & 0x03ffffff);
  }
}

namespace {
  class V810AsmBackend : public MCAsmBackend {
  private:
    Triple::OSType OSType;
  public:
    V810AsmBackend(Triple::OSType OSType) : MCAsmBackend(support::little), OSType(OSType) {}

    unsigned getNumFixupKinds() const override {
      return V810::NumTargetFixupKinds;
    }

    std::optional<MCFixupKind> getFixupKind(StringRef Name) const override {
      unsigned Type;
      Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/V810.def"
#undef ELF_RELOC
                 .Case("BFD_RELOC_NONE", ELF::R_V810_NONE)
                 .Default(-1u);
      if (Type == -1u)
        return std::nullopt;
      return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
    }

    const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
      const static MCFixupKindInfo Infos[V810::NumTargetFixupKinds] = {
        // name                  offset bits  flags
        { "fixup_v810_lo",       16,    16,   0 },
        { "fixup_v810_hi",       16,    16,   0 },
        { "fixup_v810_9_pcrel",  0,     16,   MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_v810_26_pcrel", 0,     32,   MCFixupKindInfo::FKF_IsPCRel }
      };

      if (Kind >= FirstLiteralRelocationKind)
        return MCAsmBackend::getFixupKindInfo(FK_NONE);
      if (Kind < FirstTargetFixupKind)
        return MCAsmBackend::getFixupKindInfo(Kind);
      assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
             "Invalid kind!");
      return Infos[Kind - FirstTargetFixupKind];
    }

    /// fixupNeedsRelaxation - Target specific predicate for whether a given
    /// fixup requires the associated instruction to be relaxed.
    bool fixupNeedsRelaxation(const MCFixup &Fixup,
                              uint64_t Value,
                              const MCRelaxableFragment *DF,
                              const MCAsmLayout &Layout) const override {
      return false;
    }

    void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                    const MCValue &Target, MutableArrayRef<char> Data,
                    uint64_t Value, bool IsResolved,
                    const MCSubtargetInfo *STI) const override {
      if (Fixup.getKind() >= FirstLiteralRelocationKind)
        return;
      Value = adjustFixupValue(Fixup.getKind(), Value);
      if (!Value) return; // Doesn't change encoding
      
      MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

      // The number of bits in the fixup mask
      auto NumBits = Info.TargetSize + Info.TargetOffset;
      auto NumBytes = (NumBits / 8) + ((NumBits % 8) == 0 ? 0 : 1);

      // Shift the value into position.
      Value <<= Info.TargetOffset;

      unsigned Offset = Fixup.getOffset();
      assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

      // For each byte of the fragment that the fixup touches, mask in the
      // bits from the fixup value.
      for (unsigned i = 0; i < NumBytes; ++i) {
        uint8_t mask = (((Value >> (i * 8)) & 0xff));
        Data[Offset + i] |= mask;
      }
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