#include "V810.h"
#include "Targets.h"

using namespace clang;
using namespace clang::targets;

const char *const V810TargetInfo::GCCRegNames[] = {
    // Integer registers
    "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",  "r10",
    "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21",
    "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
};

ArrayRef<const char *> V810TargetInfo::getGCCRegNames() const {
  return llvm::ArrayRef(GCCRegNames);
}

const TargetInfo::GCCRegAlias V810TargetInfo::GCCRegAliases[] = {
    {{"hp"}, "r2"},  {{"sp"}, "r3"},  {{"gp"}, "r4"}, {{"tp"}, "r5"},
    {{"lp"}, "r31"},
};

ArrayRef<TargetInfo::GCCRegAlias> V810TargetInfo::getGCCRegAliases() const {
  return llvm::ArrayRef(GCCRegAliases);
}

void V810TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "v810", Opts);
}

V810TargetInfo::CPUKind V810TargetInfo::getCPUKind(StringRef Name) const {
  if (Name == "vb")
    return CK_VB;
  return CK_GENERIC;
}

void V810TargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.push_back("vb");
}