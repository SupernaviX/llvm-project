#ifndef LLVM_LIB_TARGET_V810_V810ISELLOWERING_H
#define LLVM_LIB_TARGET_V810_V810ISELLOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class V810Subtarget;

  class V810TargetLowering : public TargetLowering {
  private:
    const V810Subtarget *Subtarget;
  public:
    V810TargetLowering(const TargetMachine &TM, const V810Subtarget &STI);

    SDValue
    LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &DL, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const override;

    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
                        const SDLoc &DL, SelectionDAG &DAG) const override;
  };
}

#endif