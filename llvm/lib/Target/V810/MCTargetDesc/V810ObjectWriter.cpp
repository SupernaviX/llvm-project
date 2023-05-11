#include "MCTargetDesc/V810MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"

using namespace llvm;

namespace {
  class V810ObjectWriter : public MCELFObjectTargetWriter {
  public:
    V810ObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_V800, false) {}
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
  llvm_unreachable("Shouldn't need to relocate, because no fixups");
  return 0;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createV810ObjectWriter(uint8_t OSABI) {
  return std::make_unique<V810ObjectWriter>(OSABI);
}