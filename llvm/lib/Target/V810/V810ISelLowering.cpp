#include "V810ISelLowering.h"
#include "V810RegisterInfo.h"
#include "V810Subtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
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