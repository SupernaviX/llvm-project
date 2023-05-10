#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_V810_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_V810_H

#include "Targets.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"
#include <optional>

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY V810TargetInfo : public TargetInfo {
public:
  V810TargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
    : TargetInfo(Triple) {
      resetDataLayout("e-p:32:32-i32:32");
  }

  void getTargetDefines(const LangOptions &Opts, MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return std::nullopt;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return BuiltinVaListKind::VoidPtrBuiltinVaList;
  }

  bool validateAsmConstraint(const char *&Name, TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  std::string_view getClobbers() const override {
    return "";
  }

  ArrayRef<const char *> getGCCRegNames() const override {
    return std::nullopt;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return std::nullopt;
  }

};
}
}

#endif