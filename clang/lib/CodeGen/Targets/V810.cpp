#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace clang;
using namespace clang::CodeGen;

namespace {
class V810ABIInfo : public DefaultABIInfo {
public:
  V810ABIInfo(CodeGenTypes &CGT)
      : DefaultABIInfo(CGT) {}
  
  ABIArgInfo classifyArgumentType(QualType Ty) const;
  ABIArgInfo classifyReturnType(QualType Ty) const;
  void computeInfo(CGFunctionInfo &FI) const override;
};

ABIArgInfo
V810ABIInfo::classifyArgumentType(QualType Ty) const {
  if (Ty->isStructureOrClassType()) {
    return ABIArgInfo::getDirect();
  }
  return DefaultABIInfo::classifyArgumentType(Ty);
}

ABIArgInfo
V810ABIInfo::classifyReturnType(QualType Ty) const {
  if (Ty->isStructureOrClassType()) {
    return ABIArgInfo::getDirect();
  }
  return DefaultABIInfo::classifyReturnType(Ty);
}

void V810ABIInfo::computeInfo(CGFunctionInfo &FI) const {
  if (!getCXXABI().classifyReturnType(FI))
    FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
  for (auto &Arg : FI.arguments())
    Arg.info = classifyArgumentType(Arg.type);
}

class V810TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  V810TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<V810ABIInfo>(CGT)) {}

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    if (GV->isDeclaration())
      return;
    const auto *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD) return;
    auto *Fn = cast<llvm::Function>(GV);

    if (FD->getAttr<V810InterruptAttr>())
      Fn->addFnAttr("interrupt");
  }
};
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createV810TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<V810TargetCodeGenInfo>(CGM.getTypes());
}