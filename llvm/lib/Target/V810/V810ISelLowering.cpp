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
  addRegisterClass(MVT::f32, &V810::GenRegsRegClass);
  // TODO: this is where we start massaging LLVM concepts into V810 registers/instructions/etc

  computeRegisterProperties(Subtarget->getRegisterInfo());
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

  assert(!IsVarArg); // TODO: varargs
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

  assert(RVLocs.size() < 2 && "Why are you returning multiple things");

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue OutVal = OutVals[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVal);
  }

  return Chain;
}