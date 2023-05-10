#include "V810.h"
#include "Targets.h"

using namespace clang;
using namespace clang::targets;

void V810TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "V810", Opts);
}