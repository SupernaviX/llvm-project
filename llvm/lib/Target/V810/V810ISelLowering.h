#ifndef LLVM_LIB_TARGET_V810_V810ISELLOWERING_H
#define LLVM_LIB_TARGET_V810_V810ISELLOWERING_H

#include "V810.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class V810Subtarget;

  namespace V810ISD {
  enum NodeType : unsigned {
    FIRST_NUMBER = ISD::BUILTIN_OP_END,
    HI,
    LO,
    CMP,
    FCMP,
    BCOND,
    SETF,
    SELECT_CC,
    CALL,
    RET_GLUE,
    RETI_GLUE,
    TAIL_CALL,
    SMUL_LOHI,
    UMUL_LOHI,
    SDIVREM,
    UDIVREM,
    CAXI,
  };
  }

  class V810TargetLowering : public TargetLowering {
  private:
    const V810Subtarget *Subtarget;
  public:
    V810TargetLowering(const TargetMachine &TM, const V810Subtarget &STI);

    const char *getTargetNodeName(unsigned Opcode) const override;

    void computeKnownBitsForTargetNode(const SDValue Op,
                                       KnownBits &Known,
                                       const APInt &DemandedElts,
                                       const SelectionDAG &DAG,
                                       unsigned Depth = 0) const override;

    ConstraintType getConstraintType(StringRef Constraint) const override;
    std::pair<unsigned, const TargetRegisterClass *>
    getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                 StringRef Constraint, MVT VT) const override;


    SDValue
    LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &DL, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const override;

    bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                        bool IsVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        LLVMContext &Context) const override;
    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
                        const SDLoc &DL, SelectionDAG &DAG) const override;

    SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const override;

    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

    bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;

    MachineBasicBlock *
    EmitInstrWithCustomInserter(MachineInstr &MI,
                                MachineBasicBlock *MBB) const override;
    MachineBasicBlock *
    ExpandSelectCC(MachineInstr &MI, MachineBasicBlock *MBB) const;


    bool IsEligibleForTailCallOptimization(CCState &CCInfo,
                                           CallLoweringInfo &CLI,
                                           MachineFunction &MF) const;
  };
}

#endif