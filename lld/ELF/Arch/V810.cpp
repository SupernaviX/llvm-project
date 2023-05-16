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
};
} // namespace

V810::V810() {
  
}

RelExpr V810::getRelExpr(RelType type, const Symbol &s,
                         const uint8_t *loc) const {
  switch (type) {
  case R_V810_NONE:
    return R_NONE;
  case R_V810_DISP26:
    return R_PC;
  default:
    error(getErrorLocation(loc) + "unknown relocation (" + Twine(type) +
          ") against symbol " + toString(s));
    return R_NONE;
  }
}

void V810::relocate(uint8_t *loc, const Relocation &rel,
                    uint64_t val) const {
  switch(rel.type) {
  case R_V810_DISP26:
    checkInt(loc, val, 22, rel);
    write32le(loc, (read32le(loc) & ~0x03ffffff) | (val & 0x03ffff));
    break;
  default:
    llvm_unreachable("unknown relocation");
  }
}

TargetInfo *elf::getV810TargetInfo() {
  static V810 target;
  return &target;
}
