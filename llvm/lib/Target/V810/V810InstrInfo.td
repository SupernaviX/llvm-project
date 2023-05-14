include "V810InstrFormats.td"

def simm5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;

def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [], []>;

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GenRegs, i16imm);
}

def LD_W : FormatVI<0b110011,
  (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
  "ld.w $addr, $r2",
  [(set i32:$r2, (load ADDRri:$addr))]
>;
def ST_W : FormatVI<0b110111,
  (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
  "st.w $r2, $addr",
  [(store i32:$r2, ADDRri:$addr)]
>;

let isMoveImm = 1 in
def MOVi : FormatII<0b010000,
  (outs GenRegs:$r2), (ins i32imm:$imm),
  "mov $imm, $r2",
  [(set i32:$r2, simm5:$imm)]
>;

let Constraints = "$r2 = $r1" in
def ADDri : FormatII<0b010001,
  (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
  "add $imm, $r2",
  [(set i32:$r2, (add GenRegs:$r1, simm5:$imm))]>;

let isReturn = 1, isTerminator = 1, r2 = 0 in
def JMP : FormatI<0b000110,
  (outs), (ins GenRegs:$r1),
  "jmp [$r1]",
  []
>;

// Non-instructions

// Zero immediate.
def : Pat<(i32 0), (COPY (i32 R0))>;
// Smol immediate.
def : Pat<(i32 simm5:$val),
          (MOVi imm:$val)>;