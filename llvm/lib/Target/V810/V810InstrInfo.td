include "V810InstrFormats.td"

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