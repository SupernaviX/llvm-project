#include "V810ISelLowering.h"
#include "V810.h"
#include "V810MachineFunctionInfo.h"
#include "MCTargetDesc/V810MCExpr.h"
#include "V810RegisterInfo.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"

using namespace llvm;

namespace {
  class V810CCState : public CCState {
    unsigned NumFixedParams = 0;

  public:
    V810CCState(CallingConv::ID CC, bool IsVarArg, MachineFunction &MF,
                SmallVectorImpl<CCValAssign> &locs, LLVMContext &C,
                unsigned NumFixedParams)
        : CCState(CC, IsVarArg, MF, locs, C),
          NumFixedParams(NumFixedParams) {}
    unsigned getNumFixedParams() const { return NumFixedParams; }
  };
}

#include "V810GenCallingConv.inc"

V810TargetLowering::V810TargetLowering(const TargetMachine &TM,
                                       const V810Subtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {

  setBooleanContents(BooleanContent::ZeroOrOneBooleanContent);  

  // Set up the register classes.
  addRegisterClass(MVT::i32, &V810::GenRegsRegClass);
  addRegisterClass(MVT::f32, &V810::GenRegsRegClass);

  // Handle addresses specially to make constants
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i32, Custom);
  // Handle branching specially
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::f32, Custom);

  // SELECT is just a SELECT_CC with hardcoded cond, expand it to that 
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Expand);
  // Need to branch to handle SELECT_CC
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Custom);
  // SETCC reduces nicely to CMP + SETF, so do that
  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::SETCC, MVT::f32, Custom);

  // all of these expand to our native MUL_LOHI and DIVREM opcodes
  setOperationAction(ISD::MULHU, MVT::i32, Expand);
  setOperationAction(ISD::MULHS, MVT::i32, Expand);
  setOperationAction(ISD::MUL,   MVT::i32, Expand);
  setOperationAction(ISD::SDIV,  MVT::i32, Expand);
  setOperationAction(ISD::UDIV,  MVT::i32, Expand);
  setOperationAction(ISD::SREM,  MVT::i32, Expand);
  setOperationAction(ISD::UREM,  MVT::i32, Expand);

  // Sign-extend smol types in registers with bitshifts
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAEND,   MVT::Other, Expand);
  setOperationAction(ISD::VAARG,   MVT::Other, Expand);

  // If we have native bit twiddlers, use em
  if (Subtarget->isNintendo()) {
    setOperationAction(ISD::BITREVERSE, MVT::i32, Legal);
    setOperationAction(ISD::BSWAP, MVT::i32, Legal);
  } else {
    setOperationAction(ISD::BITREVERSE, MVT::i32, Expand);
    setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  }

  setMinStackArgumentAlignment(Align(4));

  computeRegisterProperties(Subtarget->getRegisterInfo());
}

const char *V810TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((V810ISD::NodeType)Opcode) {
  case V810ISD::FIRST_NUMBER: break;
  case V810ISD::HI:           return "V810ISD::HI";
  case V810ISD::LO:           return "V810ISD::LO";
  case V810ISD::CMP:          return "V810ISD::CMP";
  case V810ISD::FCMP:         return "V810ISD::FCMP";
  case V810ISD::BCOND:        return "V810ISD::BCOND";
  case V810ISD::SETF:         return "V810ISD::SETF";
  case V810ISD::SELECT_CC:    return "V810ISD::SELECT_CC";
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
  V810MachineFunctionInfo *FuncInfo = MF.getInfo<V810MachineFunctionInfo>();

  // Figure out where the calling convention sez all the arguments live
  SmallVector<CCValAssign, 16> ArgLocs;
  V810CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext(),
                     MF.getFunction().getFunctionType()->getNumParams());
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

  // If this function is variadic, the varargs are on the stack, after any fixed arguments.
  if (IsVarArg) {
    unsigned VarArgOffset = CCInfo.getStackSize();
    FuncInfo->setVarArgsFrameIndex(VarArgOffset);
  }

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
  unsigned NumParams = CLI.CB ? CLI.CB->getFunctionType()->getNumParams() : 0;

  SmallVector<CCValAssign, 16> ArgLocs;
  V810CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs, *DAG.getContext(), NumParams);
  CCInfo.AnalyzeCallOperands(CLI.Outs, CC_V810);

  if (!CLI.IsTailCall) {
    Chain = DAG.getCALLSEQ_START(Chain, CCInfo.getStackSize(), 0, DL);
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

  // convert this into a target type now, so that legalization doesn't mess it up
  SDValue Callee = CLI.Callee;
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, PtrVT, 0, V810MCExpr::VK_V810_26_PCREL);

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

  Chain = DAG.getCALLSEQ_END(Chain, CCInfo.getStackSize(), 0, InGlue, DL);
  InGlue = Chain.getValue(1);

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

    Chain = DAG.getCopyFromReg(Chain, DL, Reg, VA.getLocVT(), InGlue).getValue(1);
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }
  
  return Chain;
}

static V810CC::CondCodes IntCondCodeToCC(ISD::CondCode CC) {
  switch (CC) {
  default: llvm_unreachable("Unknown integer condition code!");
  case ISD::SETEQ:  return V810CC::CC_E;
  case ISD::SETNE:  return V810CC::CC_NE;
  case ISD::SETLT:  return V810CC::CC_LT;
  case ISD::SETLE:  return V810CC::CC_LE;
  case ISD::SETGT:  return V810CC::CC_GT;
  case ISD::SETGE:  return V810CC::CC_GE;
  case ISD::SETULT: return V810CC::CC_C;
  case ISD::SETULE: return V810CC::CC_NH;
  case ISD::SETUGT: return V810CC::CC_H;
  case ISD::SETUGE: return V810CC::CC_NC;
  }
}

static V810CC::CondCodes FloatCondCodeToCC(ISD::CondCode CC) {
  switch (CC) {
  default: llvm_unreachable("Unknown float condition code!");
  // NB: the hardware throws an exception if either operand is a NaN,
  // so... the "ordered" and "unordered" conditions can just match
  case ISD::SETFALSE:
  case ISD::SETUO:
  case ISD::SETFALSE2:
    return V810CC::CC_NOP;
  case ISD::SETOEQ:
  case ISD::SETUEQ:
  case ISD::SETEQ:
    return V810CC::CC_E;
  case ISD::SETOGT:
  case ISD::SETUGT:
  case ISD::SETGT:
    return V810CC::CC_GT;
  case ISD::SETOGE:
  case ISD::SETUGE:
  case ISD::SETGE:
    return V810CC::CC_GE;
  case ISD::SETOLT:
  case ISD::SETULT:
  case ISD::SETLT:
    return V810CC::CC_LT;
  case ISD::SETOLE:
  case ISD::SETULE:
  case ISD::SETLE:
    return V810CC::CC_LE;
  case ISD::SETONE:
  case ISD::SETUNE:
  case ISD::SETNE:
    return V810CC::CC_NE;
  case ISD::SETO:
  case ISD::SETTRUE:
  case ISD::SETTRUE2:
    return V810CC::CC_BR;
  }
}

// Convert a global address into HI/LO pairs
static SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) {
  GlobalAddressSDNode *GN = dyn_cast<GlobalAddressSDNode>(Op);
  assert(GN);

  SDLoc DL(Op);
  SDValue HiTarget = DAG.getTargetGlobalAddress(GN->getGlobal(), DL, GN->getValueType(0), GN->getOffset(), V810MCExpr::VK_V810_HI);
  SDValue LoTarget = DAG.getTargetGlobalAddress(GN->getGlobal(), DL, GN->getValueType(0), GN->getOffset(), V810MCExpr::VK_V810_LO);

  EVT VT = Op.getValueType();
  SDValue Hi = DAG.getNode(V810ISD::HI, DL, VT, HiTarget);
  return DAG.getNode(V810ISD::LO, DL, VT, Hi, LoTarget);
}

static SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) {
  ConstantPoolSDNode *CP = dyn_cast<ConstantPoolSDNode>(Op);
  assert(CP);

  SDLoc DL(Op);
  SDValue HiTarget = DAG.getTargetConstantPool(CP->getConstVal(), CP->getValueType(0),
                                               CP->getAlign(), CP->getOffset(), V810MCExpr::VK_V810_HI);
  SDValue LoTarget = DAG.getTargetConstantPool(CP->getConstVal(), CP->getValueType(0),
                                               CP->getAlign(), CP->getOffset(), V810MCExpr::VK_V810_LO);

  EVT VT = Op.getValueType();
  SDValue Hi = DAG.getNode(V810ISD::HI, DL, VT, HiTarget);
  return DAG.getNode(V810ISD::LO, DL, VT, Hi, LoTarget);
}

// Convert a BR_CC into a cmp+bcond pair
static SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);

  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, Chain, LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, Chain, LHS, RHS);
  }

  return DAG.getNode(V810ISD::BCOND, DL, VT, Cmp, Cond, Dest, Cmp.getValue(1));
}

static SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  SDLoc DL(Op);
  EVT VT = Op.getValueType();

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  }

  return DAG.getNode(V810ISD::SELECT_CC, DL, VT, TrueVal, FalseVal, Cond, Cmp.getValue(1));
}

static SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();

  SDLoc DL(Op);
  EVT VT = Op.getValueType(); // This could return a bool if that's useful?

  SDVTList VTWithGlue = DAG.getVTList(VT, MVT::Glue);
  SDValue Cond;
  SDValue Cmp;
  if (LHS.getValueType().isFloatingPoint()) {
    Cond = DAG.getConstant(FloatCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::FCMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  } else {
    Cond = DAG.getConstant(IntCondCodeToCC(CC), DL, MVT::i32);
    Cmp = DAG.getNode(V810ISD::CMP, DL, VTWithGlue, DAG.getEntryNode(), LHS, RHS);
  }

  return DAG.getNode(V810ISD::SETF, DL, VT, Cond, Cmp.getValue(1));
}

static SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG, const V810TargetLowering &TLI) {
  MachineFunction &MF = DAG.getMachineFunction();
  V810MachineFunctionInfo *FuncInfo = MF.getInfo<V810MachineFunctionInfo>();

  SDLoc DL(Op);
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                 TLI.getPointerTy(MF.getDataLayout()));

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue V810TargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default: llvm_unreachable("Should not custom lower this!");

  case ISD::GlobalAddress:  return LowerGlobalAddress(Op, DAG);
  case ISD::ConstantPool:   return LowerConstantPool(Op, DAG);
  case ISD::BR_CC:          return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:      return LowerSELECT_CC(Op, DAG);
  case ISD::SETCC:          return LowerSETCC(Op, DAG);
  case ISD::VASTART:        return LowerVASTART(Op, DAG, *this);
  }
}

MachineBasicBlock *
V810TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  default: llvm_unreachable("No custom inserter found for instruction");
  case V810::SELECT_CC_Int:
  case V810::SELECT_CC_Float:
    return ExpandSelectCC(MI, BB);
  case V810::CALL_INDIRECT:
    return ExpandCallIndirect(MI, BB);
  }  
}

// This is called after LowerSELECT_CC.
// It turns the target-specific SELECT_CC instr into a set of branches.
MachineBasicBlock *
V810TargetLowering::ExpandSelectCC(MachineInstr &MI, MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget->getInstrInfo();
  DebugLoc dl = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register TrueSrc = MI.getOperand(1).getReg();
  Register FalseSrc = MI.getOperand(2).getReg();
  unsigned CC = (V810CC::CondCodes)MI.getOperand(3).getImm();

  /*
    Split this block into the following control flow structure:
    ThisMBB
    |  \
    |   IfFalseMBB
    |  /
    SinkMBB

    ThisMBB ends with a Bcond; it goes to SinkMBB if true and IfFalseMBB if false.
    IfFalseMBB falls through to SinkMBB.
    SinkMBB uses a phi node to track the result.
  */
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = ++BB->getIterator();

  MachineBasicBlock *ThisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *IfFalseMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *SinkMBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(It, IfFalseMBB);
  F->insert(It, SinkMBB);

  // Every node after the SELECT_CC in ThisMBB moves to SinkMBB
  SinkMBB->splice(SinkMBB->begin(), ThisMBB,
                  std::next(MachineBasicBlock::iterator(MI)), ThisMBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(ThisMBB);

  // Add the two successors of ThisMBB
  ThisMBB->addSuccessor(IfFalseMBB);
  ThisMBB->addSuccessor(SinkMBB);

  // ThisMBB ends with a Bcond
  BuildMI(ThisMBB, dl, TII.get(V810::Bcond))
    .addImm(CC)
    .addMBB(SinkMBB);

  // IfFalseMBB has no logic, it just falls through to SinkMBB...
  IfFalseMBB->addSuccessor(SinkMBB);

  // because the logic is handled with a phi node at SinkMBB's start.
  // %Result = phi [ %TrueValue, ThisMBB ], [ %FalseValue, IfFalseMBB ]
  BuildMI(*SinkMBB, SinkMBB->begin(), dl, TII.get(V810::PHI), Dest)
    .addReg(TrueSrc).addMBB(ThisMBB)
    .addReg(FalseSrc).addMBB(IfFalseMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone.
  return SinkMBB;
}

// Turns the target-specific CALL_INDIRECT into a "set r31 to the return address and JMP"
MachineBasicBlock *
V810TargetLowering::ExpandCallIndirect(MachineInstr &MI, MachineBasicBlock *BB) const {
  const V810InstrInfo &TII = *Subtarget->getInstrInfo();
  DebugLoc dl = MI.getDebugLoc();

  Register Target = MI.getOperand(0).getReg();

  MachineBasicBlock *ThisMBB = BB;
  MachineBasicBlock::instr_iterator I = std::next(MachineBasicBlock::instr_iterator(MI));

  // JAL to right after this instr
  BuildMI(*ThisMBB, I, dl, TII.get(V810::JAL))
    .addImm(4);
  // Fix lp to point to after this + the next instruction
  BuildMI(*ThisMBB, I, dl, TII.get(V810::ADDri), V810::R31)
    .addReg(V810::R31)
    .addImm(4);
  // And jump!
  BuildMI(*ThisMBB, I, dl, TII.get(V810::JMP))
    .addReg(Target);

  MI.eraseFromParent(); // The pseudo instruction is gone.
  return ThisMBB;
}

V810TargetLowering::ConstraintType
V810TargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:  break;
    case 'r':
      return C_RegisterClass;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
V810TargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                  StringRef Constraint,
                                                  MVT VT) const {
  if (Constraint.empty())
    return std::make_pair(0U, nullptr);
  if (Constraint == "r") {
    return std::make_pair(0U, &V810::GenRegsRegClass);
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}