#include "Symbols.h"
#include "SyntheticSections.h"
#include "Target.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class V810 final : public TargetInfo {
public:
  V810();
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
  int64_t getImplicitAddend(const uint8_t *buf, RelType type) const override;
};
} // namespace

V810::V810() {
  
}

RelExpr V810::getRelExpr(RelType type, const Symbol &s,
                         const uint8_t *loc) const {
  switch (type) {
  case R_V810_NONE:
    return R_NONE;
  case R_V810_8:
  case R_V810_16:
  case R_V810_32:
  case R_V810_LO:
  case R_V810_HI:
    return R_ABS;
  case R_V810_SDAOFF:
    return R_V810_GP;
  case R_V810_9_PCREL:
  case R_V810_26_PCREL:
    return R_PC;
  default:
    error(getErrorLocation(loc) + "unknown relocation (" + Twine(type) +
          ") against symbol " + toString(s));
    return R_NONE;
  }
}

// little-endian, but first and last halfwords are swapped
static uint32_t read32vb(const uint8_t *loc) {
  return static_cast<uint32_t>(read16le(loc) << 16)
      + static_cast<uint32_t>(read16le(loc + 2));
}
static void write32vb(uint8_t *loc, uint32_t val) {
  write16le(loc, static_cast<uint16_t>(val >> 16));
  write16le(loc + 2, static_cast<uint16_t>(val));
}

void V810::relocate(uint8_t *loc, const Relocation &rel,
                    uint64_t val) const {
  switch(rel.type) {
  case R_V810_LO:
    checkInt(loc, val, 32, rel);
    write16le(loc + 2, val & 0x0000ffff);
    break;
  case R_V810_HI:
    checkInt(loc, val, 32, rel);
    // add one to HI if LO would be negative
    write16le(loc + 2, ((val >> 16) & 0x0000ffff) + ((val & 0x8000) != 0));
    break;
  case R_V810_SDAOFF:
    checkInt(loc, val, 16, rel);
    write16le(loc + 2, val);
    break;
  case R_V810_9_PCREL:
    checkInt(loc, val, 9, rel);
    write16le(loc, (read16le(loc) & ~0x01ff) | (val & 0x01ff));
    break;
  case R_V810_26_PCREL:
    checkInt(loc, val, 26, rel);
    write32vb(loc, (read32vb(loc) & ~0x03ffffff) | (val & 0x03ffffff));
    break;
  default:
    llvm_unreachable("unknown relocation");
  }
}

int64_t V810::getImplicitAddend(const uint8_t *buf, RelType type) const {
  switch (type) {
  case R_V810_9_PCREL:
    return SignExtend64<9>(read16le(buf + 2));
  case R_V810_26_PCREL:
    return SignExtend64<26>(read32vb(buf));
  case R_V810_LO:
  case R_V810_HI:
  case R_V810_SDAOFF:
  case R_V810_NONE:
    return 0;
  default:
    internalLinkerError(getErrorLocation(buf),
                        "cannot read addend for relocation " + toString(type));
    return 0;
  }
}

TargetInfo *elf::getV810TargetInfo() {
  static V810 target;
  return &target;
}
