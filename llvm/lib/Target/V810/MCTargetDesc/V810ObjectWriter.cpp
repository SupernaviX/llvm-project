#include "MCTargetDesc/V810FixupKinds.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"

using namespace llvm;

namespace {
  class V810ObjectWriter : public MCELFObjectTargetWriter {
  public:
    V810ObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_V810, false) {}
    ~V810ObjectWriter() override = default;
  protected:
    unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                          const MCFixup &Fixup, bool IsPCRel) const override;
  };
}

unsigned V810ObjectWriter::getRelocType(MCContext &Ctx,
                                        const MCValue &Target,
                                        const MCFixup &Fixup,
                                        bool IsPCRel) const {
  MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;
  
  if (const V810MCExpr *VExpr = dyn_cast<V810MCExpr>(Fixup.getValue())) {
    if (VExpr->getKind() == V810MCExpr::VK_V810_26_PCREL) {
      return ELF::R_V810_26_PCREL;
    }
  }
  switch(Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unimplemented fixup -> relocation");
  case FK_NONE:                   return ELF::R_V810_NONE;
  case V810::fixup_v810_lo:       return ELF::R_V810_LO;
  case V810::fixup_v810_hi:       return ELF::R_V810_HI;
  case V810::fixup_v810_26_pcrel: return ELF::R_V810_26_PCREL;
  }

  return ELF::R_V810_NONE;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createV810ObjectWriter(uint8_t OSABI) {
  return std::make_unique<V810ObjectWriter>(OSABI);
}