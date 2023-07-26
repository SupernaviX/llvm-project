class CCIfArgIsVarArg<CCAction A>
  : CCIf<"State.isVarArg() && "
         "ValNo >= static_cast<V810CCState&>(State).getNumFixedParams()",
         A>;

def CC_V810 : CallingConv<[
  CCIfArgIsVarArg<CCAssignToStack<4, 4>>,
  CCIfType<[i8, i16, i32, f32], CCAssignToReg<[R6, R7, R8, R9]>>,
  CCAssignToStack<4, 4>
]>;

def RetCC_V810 : CallingConv<[
  CCIfType<[i8, i16, i32, f32], CCAssignToReg<[R10]>>
]>;

def CSR : CalleeSavedRegs<(add R31, (sequence "R%u", 2, 5))>;

def CSR_Interrupt : CalleeSavedRegs<(sequence "R%u", 1, 31)>;