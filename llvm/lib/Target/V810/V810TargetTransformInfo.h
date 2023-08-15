#ifndef LLVM_LIB_TARGET_V810_V810TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_V810_V810TARGETTRANSFORMINFO_H

#include "V810Subtarget.h"
#include "V810TargetMachine.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {
class V810TTIImpl : public BasicTTIImplBase<V810TTIImpl> {
  using BaseT = BasicTTIImplBase<V810TTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const V810Subtarget *ST;
  const V810TargetLowering *TLI;

  const V810Subtarget *getST() const { return ST; }
  const V810TargetLowering *getTLI() const { return TLI; }

public:
  explicit V810TTIImpl(const V810TargetMachine *TM, const Function &F)
    : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
      TLI(ST->getTargetLowering()) {}

  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE);
};
}

#endif