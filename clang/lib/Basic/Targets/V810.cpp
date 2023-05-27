#include "V810.h"
#include "Targets.h"

using namespace clang;
using namespace clang::targets;

void V810TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "V810", Opts);
}

V810TargetInfo::CPUKind V810TargetInfo::getCPUKind(StringRef Name) const {
  if (Name == "vb")
    return CK_VB;
  return CK_GENERIC;
}

void V810TargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.push_back("vb");
}