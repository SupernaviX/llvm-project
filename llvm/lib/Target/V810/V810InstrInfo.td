include "V810InstrFormats.td"

def simm5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;

def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [], []>;

def MEMri : Operand<iPTR> {
  let MIOperandInfo = (ops GenRegs, i16imm);
}

def LD_W : FormatVI<0b110011,
  (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
  "ld.w $addr, $r2",
  [(set i32:$r2, (load ADDRri:$addr))]
>;
def ST_W : FormatVI<0b110111,
  (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
  "st.w $addr, $r2",
  [(store i32:$r2, ADDRri:$addr)]
>;

let isMoveImm = 1 in
def MOVi : FormatII<0b010000,
  (outs GenRegs:$r2), (ins i32imm:$r1),
  "mov $r2, $r1",
  [(set i32:$r2, simm5:$r1)]
>;

// Non-instructions

// Zero immediate.
def : Pat<(i32 0), (COPY (i32 R1))>;
