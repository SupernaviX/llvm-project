include "V810InstrFormats.td"

// split a 32-bit immediate into low and high bits
def IMM32_LO16 : SDNodeXForm<imm, [{
  unsigned sextValue = (unsigned)N->getSExtValue();
  return CurDAG->getTargetConstant(sextValue & 0xffff, SDLoc(N), MVT::i32);
}]>;
def IMM32_HI16 : SDNodeXForm<imm, [{
  unsigned sextValue = (unsigned)N->getSExtValue();
  // it's implementation-dependent whether right-shifting a negative is sext or zext,
  // but we don't care because we're truncating it anyway.
  unsigned hiBits = (sextValue >> 16) & 0xffff;
  if (sextValue & 0x00008000) {
    // Low bits are sign-extended, high bits are not.
    // If the low 16 bits would sign-extend negative, compensate by incrementing high.
    hiBits = (hiBits + 1) & 0xffff;
  }
  return CurDAG->getTargetConstant(hiBits, SDLoc(N), MVT::i32);
}]>;

// grab the high 16 bits from a 32-bit immediate
def HI16 : SDNodeXForm<imm, [{
  // it's implementation-dependent whether right-shifting a negative is sext or zext,
  // but we don't care because we're truncating it anyway.
  return CurDAG->getTargetConstant(((unsigned)N->getSExtValue() >> 16) & 0xffff, SDLoc(N),
                                   MVT::i32);
}]>;

def simm5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;
def simm16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;
def MOVHIimm : PatLeaf<(imm), [{
  return isShiftedUInt<16, 16>(N->getZExtValue());
}], HI16>;

def ADDR : ComplexPattern<iPTR, 1, "SelectADDR", [], []>;
def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [], []>;

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GenRegs, i16imm);
}
def calltarget : Operand<iPTR> {
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
}
def brtarget : Operand<OtherVT>;

def SDT_V810Call  : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;
def call          : SDNode<"V810ISD::CALL", SDT_V810Call,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                            SDNPVariadic]>;
def tailcall      : SDNode<"V810ISD::TAIL_CALL", SDT_V810Call,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                            SDNPVariadic]>;
def SDT_V810Ret   : SDTypeProfile<0, 0, []>;
def retglue       : SDNode<"V810ISD::RET_GLUE", SDT_V810Ret,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let mayLoad = 1 in {
  def LD_W : FormatVI<0b110011,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.w $addr, $r2",
    [(set i32:$r2, (load ADDRri:$addr))]
  >;
  def IN_W : FormatVI<0b111011,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.w $addr, $r2",
    [(set i32:$r2, (load ADDRri:$addr))]
  >;
  def LD_H : FormatVI<0b110001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.h $addr, $r2",
    [(set i32:$r2, (sextloadi16 ADDRri:$addr))]
  >;
  def IN_H : FormatVI<0b111001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.h $addr, $r2",
    [(set i32:$r2, (zextloadi16 ADDRri:$addr))]
  >;
  def LD_B : FormatVI<0b110000,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "ld.b $addr, $r2",
    [(set i32:$r2, (sextloadi8 ADDRri:$addr))]
  >;
  def IN_B : FormatVI<0b111000,
    (outs GenRegs:$r2), (ins (MEMri $r1, $disp):$addr),
    "in.b $addr, $r2",
    [(set i32:$r2, (zextloadi8 ADDRri:$addr))]
  >;
}

let mayStore = 1 in {
  def ST_W : FormatVI<0b110111,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.w $r2, $addr",
    [(store i32:$r2, ADDRri:$addr)]
  >;
  def OUT_W : FormatVI<0b111111,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.w $r2, $addr",
    [(store i32:$r2, ADDRri:$addr)]
  >;
  def ST_H : FormatVI<0b110101,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.h $r2, $addr",
    [(truncstorei16 i32:$r2, ADDRri:$addr)]
  >;
  def OUT_H : FormatVI<0b111101,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.h $r2, $addr",
    [(truncstorei16 i32:$r2, ADDRri:$addr)]
  >;
  def ST_B : FormatVI<0b110100,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "st.b $r2, $addr",
    [(truncstorei8 i32:$r2, ADDRri:$addr)]
  >;
  def OUT_B : FormatVI<0b111100,
    (outs), (ins (MEMri $r1, $disp):$addr, GenRegs:$r2),
    "out.b $r2, $addr",
    [(truncstorei8 i32:$r2, ADDRri:$addr)]
  >;
}

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
    [] // no pattern matching because this is very situational
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
  [(set i32:$r2, (add GenRegs:$r2_, GenRegs:$r1))]
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

let Constraints = "$r2 = $r2_" in
def SUB : FormatI<0b000010,
  (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
  "sub $r1, $r2",
  [(set i32:$r2, (sub GenRegs:$r2_, GenRegs:$r1))]
>;

let isTerminator = 1, isBarrier = 1 in {
  let r2 = 0 in
  def JMP : FormatI<0b000110,
    (outs), (ins GenRegs:$r1),
    "jmp [$r1]",
    []
  >;

  def JR : FormatIV<0b101010,
    (outs), (ins brtarget:$disp),
    "jr $disp",
    [(br bb:$disp)]
  >;

  let Defs=[R31] in
  def JAL : FormatIV<0b101011,
    (outs), (ins calltarget:$disp),
    "jal $disp",
    []
  >;
}


let isCodeGenOnly = 1 in {
  let isCall = 1, Defs = [R31] in
  def CALL : FormatIV<0b101011,
    (outs), (ins calltarget:$disp),
    "jal $disp",
    [(call ADDR:$disp)]
  >;

  let isReturn = 1, isTerminator = 1, isBarrier = 1, r2 = 0, r1 = 31 in
  def RET : FormatI<0b000110,
    (outs), (ins),
    "jmp [r31]",
    [(retglue)]
  >;
}



// Non-instructions

// Zero immediate.
def : Pat<(i32 0), (COPY (i32 R0))>;
// Smol immediate (up to 5 bits).
def : Pat<(i32 simm5:$val),
          (MOVi imm:$val)>;
// Med immediate (up to 16 bits).
def : Pat<(i32 simm16:$val),
          (MOVEA R0, imm:$val)>;
// Immediate that only uses the top 16 bits
def : Pat<(i32 MOVHIimm:$val),
          (MOVHI R0, (HI16 imm:$val))>;
// Lorg immediate (up to 32 bits).
def : Pat<(i32 imm:$val),
          (MOVEA (MOVHI R0, (IMM32_HI16 imm:$val)), (IMM32_LO16 imm:$val))>;