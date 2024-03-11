#include "V810TargetMachine.h"
#include "llvm/CodeGen/SelectionDAG.h"
using namespace llvm;

#define DEBUG_TYPE "v810-selectiondag-info"

SDValue V810SelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile, bool AlwaysInline,
    MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo) const {
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Size);
  if (AlwaysInline || Alignment < Align(4) || !ConstantSize) {
    return SDValue();
  }

  uint64_t SizeVal = ConstantSize->getZExtValue();
  if ((SizeVal % 4) != 0) {
    return SDValue();
  }

  const TargetLowering &TLI = *DAG.getSubtarget().getTargetLowering();
  TargetLowering::ArgListTy Args;
  TargetLowering::ArgListEntry Entry;
  Entry.Ty = DAG.getDataLayout().getIntPtrType(*DAG.getContext());
  Entry.Node = Dst;
  Args.push_back(Entry);
  Entry.Node = Src;
  Args.push_back(Entry);
  Entry.Node = Size;
  Args.push_back(Entry);

  const char *SpecialMemcpyName = "__memcpy_wordaligned";

  TargetLowering::CallLoweringInfo CLI(DAG);
  CLI.setDebugLoc(dl)
      .setChain(Chain)
      .setLibCallee(
          TLI.getLibcallCallingConv(RTLIB::MEMCPY),
          Type::getVoidTy(*DAG.getContext()),
          DAG.getTargetExternalSymbol(
              SpecialMemcpyName, TLI.getPointerTy(DAG.getDataLayout()), 0),
          std::move(Args))
      .setDiscardResult();
  
  std::pair<SDValue, SDValue> CallResult = TLI.LowerCallTo(CLI);
  return CallResult.second;
}