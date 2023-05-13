def CC_V810 : CallingConv<[
  CCIfType<[i16, i32, f32], CCAssignToReg<[R6, R7, R8, R9]>>,
  CCAssignToStack<4, 4>
]>;

def RetCC_V810 : CallingConv<[
  CCIfType<[i16, i32, f32], CCAssignToReg<[R10]>>
]>;

def CSR : CalleeSavedRegs<(sequence "R%u", 2, 5)>;
