#include "V810TargetTransformInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
using namespace llvm;

#define DEBUG_TYPE "v810tti"

void V810TTIImpl::getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                                          TTI::UnrollingPreferences &UP,
                                          OptimizationRemarkEmitter *ORE) {
  // Only unroll loops with VERY small bodies
  UP.Threshold = 3;
}