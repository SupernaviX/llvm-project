include "V810InstrFormats.td"

def IsNintendo : Predicate<"Subtarget->isNintendo()">,
                 AssemblerPredicate<(all_of FeatureNintendo)>;

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

def uimm5 : PatLeaf<(imm), [{ return isInt<5>(N->getZExtValue()); }]>;
def simm5 : PatLeaf<(imm), [{ return isInt<5>(N->getSExtValue()); }]>;
def simm16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;
def MOVHIimm : PatLeaf<(imm), [{
  return isShiftedUInt<16, 16>(N->getZExtValue());
}], HI16>;

def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [frameindex], []>;

def V810MEMriAsmOperand : AsmOperandClass {
  let Name = "MEMri";
  let ParserMethod = "parseMEMOperand";
}

def V810BranchTargetAsmOperand : AsmOperandClass {
  let Name = "BranchTarget";
  let ParserMethod = "parseBranchTargetOperand";
}

def V810BcondTargetAsmOperand : AsmOperandClass {
  let Name = "BcondTarget";
  let ParserMethod = "parseBcondTargetOperand";
}

def V810CondAsmOperand : AsmOperandClass {
  let Name = "Cond";
  let ParserMethod = "parseCondOperand";
}

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GenRegs, i16imm);
  let ParserMatchClass = V810MEMriAsmOperand;
}
def calltarget : Operand<iPTR> {
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "getBranchTargetOpValue";
  let ParserMatchClass = V810BranchTargetAsmOperand;
}
def brtarget : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "getBranchTargetOpValue";
  let ParserMatchClass = V810BranchTargetAsmOperand;
}
def bcondtarget : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
  let EncoderMethod = "getBcondTargetOpValue";
  let ParserMatchClass = V810BcondTargetAsmOperand;
}
def condcode : Operand<i32> {
  let PrintMethod = "printCondOperand";
  let ParserMatchClass = V810CondAsmOperand;
}

def V810Hi  : SDNode<"V810ISD::HI", SDTIntUnaryOp>;
def V810Lo  : SDNode<"V810ISD::LO", SDTIntBinOp>;

def SDT_V810Cmp       : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>, SDTCisInt<0>]>;
def V810CMP           : SDNode<"V810ISD::CMP", SDT_V810Cmp, [SDNPHasChain, SDNPOutGlue]>;
def SDT_V810Bcond     : SDTypeProfile<0, 2, [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
def V810Bcond         : SDNode<"V810ISD::BCOND", SDT_V810Bcond, [SDNPHasChain]>;
def SDT_V810Setf      : SDTypeProfile<1, 1, [SDTCisInt<0>, SDTCisInt<1>]>;
def V810Setf          : SDNode<"V810ISD::SETF", SDT_V810Setf, [SDNPInGlue]>;
def SDT_V810Selectcc  : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>, SDTCisVT<3, i32>]>;
def V810Selectcc      : SDNode<"V810ISD::SELECT_CC", SDT_V810Selectcc, [SDNPInGlue]>;

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

def SDT_SPCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_SPCallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_SPCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_SPCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

// Conditions
// Keep this in sync with the values in V810.h
class ICC_VAL<int N> : PatLeaf<(i32 N)>;
def ICC_V   : ICC_VAL<0>; // Overflow
def ICC_C   : ICC_VAL<1>; // Carry/Lower (unsigned)
def ICC_E   : ICC_VAL<2>; // Equal/Zero
def ICC_NH  : ICC_VAL<3>; // Not higher (unsigned)
def ICC_N   : ICC_VAL<4>; // Negative
def ICC_BR  : ICC_VAL<5>; // Always (unconditional branch)
def ICC_LT  : ICC_VAL<6>; // Less than (signed)
def ICC_LE  : ICC_VAL<7>; // Less than or equal (signed)
def ICC_NV  : ICC_VAL<8>; // Not overflow
def ICC_NC  : ICC_VAL<9>; // Not carry/lower (unsigned)
def ICC_NE  : ICC_VAL<10>; // Not equal/zero
def ICC_H   : ICC_VAL<11>; // Nigher (unsigned)
def ICC_P   : ICC_VAL<12>; // Positive
def ICC_NOP : ICC_VAL<13>; // Never (nop)
def ICC_GE  : ICC_VAL<14>; // Greater than or equal (signed)
def ICC_GT  : ICC_VAL<15>; // Greater than

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

// loads that aren't explicitly sign-extended or zero-extended will sign-extend
def : Pat<(i32 (extloadi16 ADDRri:$addr)), (LD_H ADDRri:$addr)>;
def : Pat<(i32 (extloadi8 ADDRri:$addr)), (LD_B ADDRri:$addr)>;

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

// Bit ops
let Defs = [SR5] in {
  def ANDI : FormatV<0b101101,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "andi $imm, $r1, $r2",
    [(set i32:$r2, (and GenRegs:$r1, simm16:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def AND : FormatI<0b001101,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "and $r1, $r2",
    [(set i32:$r2, (and GenRegs:$r2_, GenRegs:$r1))]
  >;

  def ORI : FormatV<0b101100,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "ori $imm, $r1, $r2",
    [(set i32:$r2, (or GenRegs:$r1, simm16:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def OR : FormatI<0b001100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "or $r1, $r2",
    [(set i32:$r2, (or GenRegs:$r2_, GenRegs:$r1))]
  >;

  def XORI : FormatV<0b101110,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "xori $imm, $r1, $r2",
    [(set i32:$r2, (xor GenRegs:$r1, simm16:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def XOR : FormatI<0b001110,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "xor $r1, $r2",
    [(set i32:$r2, (xor GenRegs:$r2_, GenRegs:$r1))]
  >;

  def NOT : FormatI<0b001111,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "not $r1, $r2",
    [(set i32:$r2, (not GenRegs:$r1))]
  >;

  let Constraints = "$r2 = $r1" in
  def SHLi : FormatII<0b010100,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "shl $imm, $r2",
    [(set i32:$r2, (shl GenRegs:$r1, uimm5:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def SHLr : FormatI<0b000100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "shl $r1, $r2",
    [(set i32:$r2, (shl GenRegs:$r2_, GenRegs:$r1))]
  >;

  let Constraints = "$r2 = $r1" in
  def SHRi : FormatII<0b010101,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "shr $imm, $r2",
    [(set i32:$r2, (srl GenRegs:$r1, uimm5:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def SHRr : FormatI<0b000101,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "shr $r1, $r2",
    [(set i32:$r2, (srl GenRegs:$r2_, GenRegs:$r1))]
  >;

  let Constraints = "$r2 = $r1" in
  def SARi : FormatII<0b010111,
    (outs GenRegs:$r2), (ins GenRegs:$r1, i32imm:$imm),
    "sar $imm, $r2",
    [(set i32:$r2, (sra GenRegs:$r1, uimm5:$imm))]
  >;
  let Constraints = "$r2 = $r2_" in
  def SARr : FormatI<0b000111,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "sar $r1, $r2",
    [(set i32:$r2, (sra GenRegs:$r2_, GenRegs:$r1))]
  >;
}

// Maths

let Defs = [SR5] in {
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

  def CMPri : FormatII<0b010011,
    (outs), (ins GenRegs:$r2, i32imm:$imm),
    "cmp $imm, $r2",
    [(V810CMP GenRegs:$r2, simm5:$imm)]
  >;
  def CMPrr : FormatI<0b000011,
    (outs), (ins GenRegs:$r2, GenRegs:$r1),
    "cmp $r1, $r2",
    [(V810CMP GenRegs:$r2, GenRegs:$r1)]
  >;

  // Support loading addresses into a register
  let isCodeGenOnly = 1 in
  def ADDImem : FormatV<0b101001,
    (outs GenRegs:$r2), (ins (MEMri $r1, $imm):$addr),
    "addi $addr, $r2",
    [(set iPTR:$r2, ADDRri:$addr)]
  >;
}

// These hi+lo instructions have return values handled in code
let Defs = [R30, SR5], Constraints = "$r2 = $rlo" in {
  def MUL : FormatI<0b001000,
    (outs GenRegs:$rlo, GenRegs:$rhi), (ins GenRegs:$r2, GenRegs:$r1),
    "mul $r1, $r2",
    []
  >;
  def MULU: FormatI<0b001010,
    (outs GenRegs:$rlo, GenRegs:$rhi), (ins GenRegs:$r2, GenRegs:$r1),
    "mulu $r1, $r2",
    []
  >;
}
let Defs = [R30, SR5], Constraints = "$r2 = $rquot" in {
  def DIV : FormatI<0b001001,
    (outs GenRegs:$rquot, GenRegs:$rrem), (ins GenRegs:$r2, GenRegs:$r1),
    "div $r1, $r2",
    []
  >;
  def DIVU : FormatI<0b001011,
    (outs GenRegs:$rquot, GenRegs:$rrem), (ins GenRegs:$r2, GenRegs:$r1),
    "divu $r1, $r2",
    []
  >;
}

let Uses = [SR5], usesCustomInserter = 1 in
def SELECT_CC_Int : Pseudo<
  (outs GenRegs:$dst), (ins GenRegs:$t, GenRegs:$f, condcode:$cond),
  "; SELECT_CC_Int PSEUDO!",
  [(set i32:$dst, (V810Selectcc i32:$t, i32:$f, imm:$cond))]
>;

// conditionals
// V810ISelLowering generates the patterns which these instrs match, when lowering
// LLVM conditional forms. That's because CMP is an instruction which writes SR5
// flags that BCond/SETF read, and idk how to express that dependency in tablegen.
let Uses = [SR5] in {
  let isTerminator = 1, isBranch = 1 in
  def Bcond : FormatIII<0b100,
    (outs), (ins condcode:$cond, bcondtarget:$disp),
    "b$cond $disp",
    [(V810Bcond imm:$cond, bb:$disp)]
  >;

  def SETF : FormatII<0b010010,
    (outs GenRegs:$r2), (ins condcode:$imm),
    "setf $imm, $r2",
    [(set i32:$r2, (V810Setf imm:$imm))]
  >;
}

let isTerminator = 1, isBarrier = 1 in {
  let r2 = 0 in
  def JMP : FormatI<0b000110,
    (outs), (ins GenRegs:$r1),
    "jmp $r1",
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

// Function call tools

let isCodeGenOnly = 1 in {
  let isCall = 1, Defs = [R31] in
  def CALL : FormatIV<0b101011,
    (outs), (ins calltarget:$disp, variable_ops),
    "jal $disp",
    []
  >;

  let isReturn = 1, isTerminator = 1, isBarrier = 1, r2 = 0, r1 = 31 in
  def RET : FormatI<0b000110,
    (outs), (ins),
    "jmp [r31]",
    [(retglue)]
  >;

  let Defs = [R3], Uses = [R3], hasSideEffects = 1 in {
    def ADJCALLSTACKDOWN : Pseudo<
      (outs), (ins i32imm:$amt1, i32imm:$amt2),
      "!ADJCALLSTACKDOWN $amt1, $amt2",
      [(callseq_start timm:$amt1, timm:$amt2)]
    >;
    def ADJCALLSTACKUP : Pseudo<
      (outs), (ins i32imm:$amt1, i32imm:$amt2),
      "!ADJCALLSTACKUP $amt1, $amt2",
      [(callseq_end timm:$amt1, timm:$amt2)]
    >;
  }
}

def : Pat<(call tglobaladdr:$dst),
          (CALL tglobaladdr:$dst)>;

// System stuff
def LDSR : FormatII<0b011100,
  (outs SysRegs:$imm), (ins GenRegs:$r2),
  "ldsr $r2, $imm",
  []
>;
def STSR : FormatII<0b011101,
  (outs GenRegs:$r2), (ins SysRegs:$imm),
  "stsr $imm, $r2",
  []
>;

let r2 = 0, imm = 0 in {
  def HALT : FormatII<0b011010,
    (outs), (ins),
    "halt",
    []
  >;
  def RETI : FormatII<0b011001,
    (outs), (ins),
    "reti",
    []
  >;
}

let r2 = 0 in
def TRAP : FormatII<0b011000,
  (outs), (ins i32imm:$imm),
  "trap $imm",
  []
>;
def : Pat<(trap), (TRAP 0)>;

// VB-specific instructions
let Predicates = [IsNintendo] in {
  let Defs = [SR5], r2 = 0, imm = 0 in {
    def CLI : FormatII<0b010110,
      (outs), (ins),
      "cli",
      []
    >;
    def SEI : FormatII<0b011110,
      (outs), (ins),
      "sei",
      []
    >;
  }

  let Constraints = "$r2 = $r2_" in
  def MPYHW : FormatVII<0b001100,
    (outs GenRegs:$r2), (ins GenRegs:$r2_, GenRegs:$r1),
    "mpyhw $r1, $r2",
    []
  >;

  def REV : FormatVII<0b001010,
    (outs GenRegs:$r2), (ins GenRegs:$r1),
    "rev $r1, $r2",
    [(set i32:$r2, (bitreverse GenRegs:$r1))]
  >;

  let Constraints = "$r2 = $r2_", r1 = 0 in
  def XB : FormatVII<0b001000,
    (outs GenRegs:$r2), (ins GenRegs:$r2_),
    "xb $r2",
    []
  >;

  let Constraints = "$r2 = $r2_", r1 = 0 in
  def XH : FormatVII<0b001001,
    (outs GenRegs:$r2), (ins GenRegs:$r2_),
    "xh $r2",
    []
  >;

  def : Pat<(i32 (bswap GenRegs:$r2)),
            (XB (XH (XB GenRegs:$r2)))>;

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

// Make global addresses work
def : Pat<(V810Lo (V810Hi tglobaladdr:$inhi), tglobaladdr:$inlo),
          (MOVEA (MOVHI R0, tglobaladdr:$inhi), tglobaladdr:$inlo)>;

include "V810InstrAliases.td"