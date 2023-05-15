#ifndef LLVM_LIB_TARGET_V810_V810ISELLOWERING_H
#define LLVM_LIB_TARGET_V810_V810ISELLOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class V810Subtarget;

  namespace V810ISD {
  enum NodeType : unsigned {
    FIRST_NUMBER = ISD::BUILTIN_OP_END,
    CALL,
    RET_GLUE,

    TAIL_CALL,
  };
  }

  class V810TargetLowering : public TargetLowering {
  private:
    const V810Subtarget *Subtarget;
  public:
    V810TargetLowering(const TargetMachine &TM, const V810Subtarget &STI);

    const char *getTargetNodeName(unsigned Opcode) const override;

    SDValue
    LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &DL, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const override;

    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
                        const SDLoc &DL, SelectionDAG &DAG) const override;

    SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const override;
  };
}

#endif