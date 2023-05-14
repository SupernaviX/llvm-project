include "V810InstrFormats.td"

def simm5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;
def simm16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

def LO16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((unsigned)N->getSExtValue() & 0xffff, SDLoc(N),
                                   MVT::i32);
}]>;
def HI16 : SDNodeXForm<imm, [{
  // it's implementation-dependent whether right-shifting a negative is sext or zext,
  // but we don't care because we're truncating it anyway
  return CurDAG->getTargetConstant(((unsigned)N->getSExtValue() >> 16) & 0xffff, SDLoc(N),
                                   MVT::i32);
}]>;
def MOVHIimm : PatLeaf<(imm), [{
  return isShiftedUInt<16, 16>(N->getZExtValue());
}], HI16>;

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
let isMoveReg = 1 in
def MOVr : FormatI<0b000000,
  (outs GenRegs:$r2), (ins GenRegs:$r1),
  "mov $r1, $r2",
  [] // no pattern matching needed because copyPhysReg handles it
>;
let isMoveImm = 1, isMoveReg = 1 in {
  def MOVHI : FormatV<0b101111,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "movhi $imm, $r1, $r2",
    [(set i32:$r2, (add GenRegs:$r1, MOVHIimm:$imm))]
  >;
  def MOVEA : FormatV<0b101000,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "movea $imm, $r1, $r2",
    [(set i32:$r2, (add GenRegs:$r1, simm16:$imm))]
  >;
}

let Constraints = "$r2 = $r2_" in
def ADDrr : FormatI<0b000001,
  (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
  "add $r1, $r2",
  [(set i32:$r2, (add GenRegs:$r1, GenRegs:$r2_))]
>;
let Constraints = "$r2 = $r1" in
def ADDri : FormatII<0b010001,
  (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
  "add $imm, $r2",
  [(set i32:$r2, (add GenRegs:$r1, simm5:$imm))]
>;
def ADDI : FormatV<0b101001,
  (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
  "addi $imm, $r1, $r2",
  [(set i32:$r2, (add GenRegs:$r1, simm16:$imm))]
>;

let isReturn = 1, isTerminator = 1, r2 = 0 in
def JMP : FormatI<0b000110,
  (outs), (ins GenRegs:$r1),
  "jmp [$r1]",
  []
>;

// Non-instructions

// Zero immediate.
def : Pat<(i32 0), (COPY (i32 R0))>;
// Smol immediate (up to 5 bits).
def : Pat<(i32 simm5:$val),
          (MOVi imm:$val)>;
// Med immediate (up to 16 bits).
def : Pat<(i32 simm16:$val),
          (MOVEA R0, imm:$val)>;
// Lorg immediate (up to 32 bits).
def : Pat<(i32 imm:$val),
          (MOVEA (MOVHI R0, (HI16 imm:$val)), (LO16 imm:$val))>;