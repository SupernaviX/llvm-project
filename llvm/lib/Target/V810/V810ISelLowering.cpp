#include "V810ISelLowering.h"
#include "V810RegisterInfo.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

using namespace llvm;

#include "V810GenCallingConv.inc"

V810TargetLowering::V810TargetLowering(const TargetMachine &TM,
                                       const V810Subtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  
  // Set up the register classes.
  addRegisterClass(MVT::i32, &V810::GenRegsRegClass);

  // Sign-extend smol types in registers with bitshifts
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  computeRegisterProperties(Subtarget->getRegisterInfo());
}

const char *V810TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((V810ISD::NodeType)Opcode) {
  case V810ISD::FIRST_NUMBER: break;
  case V810ISD::CALL:         return "V810ISD::CALL";
  case V810ISD::TAIL_CALL:    return "V810ISD::TAIL_CALL";
  case V810ISD::RET_GLUE:     return "V810ISD::RET_GLUE";
  }
  return nullptr;
}

SDValue V810TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins,
    const SDLoc &DL, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();

  // Figure out where the calling convention sez all the arguments live
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_V810);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // This argument is passed in a register.
      // The registers are all 32 bits, so...

      // Create a virtual register for the promoted live-in value.
      Register VReg = MF.addLiveIn(VA.getLocReg(),
                                   getRegClassFor(VA.getLocVT()));
      SDValue Arg = DAG.getCopyFromReg(Chain, DL, VReg, VA.getLocVT());
      if (VA.getLocInfo() == CCValAssign::SExt) {}

      // If needed, truncate the register down to the argument type
      if (VA.isExtInLoc()) {
        Arg = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Arg);
      }
      InVals.push_back(Arg);
      continue;
    }

    // If it wasn't passed as a register, it's on the stack
    assert(VA.isMemLoc());
    unsigned Offset = VA.getLocMemOffset();
    unsigned ValSize = VA.getValVT().getSizeInBits() / 8;
    int FI = MF.getFrameInfo().CreateFixedObject(ValSize, Offset, true);

    // So, read it from there
    InVals.push_back(
      DAG.getLoad(VA.getValVT(), DL, Chain,
                  DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout())),
                  MachinePointerInfo::getFixedStack(MF, FI)));
  }

  assert(!IsVarArg && "varags not supported yet");
  return Chain;
}

SDValue
V810TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_V810);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  assert(RVLocs.size() < 2 && "Why are you returning multiple things");

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue OutVal = OutVals[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVal, Glue);

    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;
  if (Glue.getNode()) {
    RetOps.push_back(Glue);
  }

  return DAG.getNode(V810ISD::RET_GLUE, DL, MVT::Other, RetOps);
}

SDValue
V810TargetLowering::LowerCall(CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  MachineFunction &MF = DAG.getMachineFunction();
  SDLoc DL = CLI.DL;
  SDValue Chain = CLI.Chain;
  MVT PtrVT = getPointerTy(DAG.getDataLayout());

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs, *DAG.getContext());

  if (!CLI.IsTailCall) {
    //Chain = DAG.getCALLSEQ_START(Chain, CCInfo.getNextStackOffset(), 0, DL);
  }

  // Collect the registers to pass in.
  // Keys are the register, values are the instructions to pass em
  SmallVector<std::pair<Register, SDValue>, 8> RegsToPass;

  // Collect all the stores that have to happen before the call
  SmallVector<SDValue, 8> MemOpChains;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    const CCValAssign &VA = ArgLocs[i];
    SDValue Arg = CLI.OutVals[i];
    // TODO: may have to do extension things here

    if (VA.isRegLoc()) {
      Register Reg = VA.getLocReg();
      RegsToPass.push_back(std::make_pair(Reg, Arg));
      continue;
    }

    // guess it was on the stack
    assert(VA.isMemLoc());
    // so pull the stack pointer,
    SDValue StackPtr = DAG.getRegister(V810::R3, PtrVT);
    // add our offset to it,
    SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
    PtrOff = DAG.getNode(ISD::ADD, DL, PtrVT, StackPtr, PtrOff);
    // and track that we need to store the value there
    MemOpChains.push_back(
        DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo()));
  }

  // Emit all the stores, make sure they occur before the call
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);
  }

  // Emit all the CopyToReg nodes, together
  SDValue InGlue;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, DL,
                             RegsToPass[i].first, RegsToPass[i].second, InGlue);
    InGlue = Chain.getValue(1);
  }

  // TODO: may need to convert these XYZ to TargetXYZ
  SDValue Callee = CLI.Callee;

  // Now build the ops for the call
  SmallVector<SDValue, 8> Ops;
  // first op is the chain, representing prereqs
  Ops.push_back(Chain);
  // second op is the callee, typically the GlobalAddress of the calling func
  Ops.push_back(Callee);
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    // next ops are the registers to pass,
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));
  }
  // then a mask describing the call-preserved registers
  const V810RegisterInfo *TRI = Subtarget->getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(DAG.getMachineFunction(),
                                                   CLI.CallConv);
  assert(Mask && "Need to specify which registers are reserved for this calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InGlue.getNode()) {
    Ops.push_back(InGlue);
  }

  if (CLI.IsTailCall) {
    DAG.getMachineFunction().getFrameInfo().setHasTailCall();
    return DAG.getNode(V810ISD::TAIL_CALL, DL, MVT::Other, Ops);
  }
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(V810ISD::CALL, DL, NodeTys, Ops);
  InGlue = Chain.getValue(1);

  //Chain = DAG.getCALLSEQ_END(Chain, CCInfo.getNextStackOffset(), 0, InGlue, DL);
  //InGlue = Chain.getValue(1);

  // Now after all that ceremony, extract the return values.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RVInfo(CLI.CallConv, CLI.IsVarArg, MF, RVLocs, *DAG.getContext());
  RVInfo.AnalyzeCallResult(CLI.Ins, RetCC_V810);

  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    // We are returning values in regs (up to one reg, really).
    // Copy frmo that register to a virtual register.
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers");
    unsigned Reg = VA.getLocReg();

    SDValue RV = DAG.getCopyFromReg(Chain, DL, Reg, VA.getLocVT(), InGlue);
    Chain = RV.getValue(1);
    InGlue = Chain.getValue(2);

    // TODO: maybe promote? maybe truncate?

    InVals.push_back(RV);
  }
  
  return Chain;
}