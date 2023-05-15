class InstV810<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
  let Namespace = "V810";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "V810";
}

// register-to-register
class FormatI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  field bits<16> Inst;
  let Size = 2;

  bits<5> r2;
  bits<5> r1;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

// immediate-to-register
class FormatII<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  field bits<16> Inst;
  let Size = 2;

  bits<5> r2;
  bits<5> imm;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = imm;
}

// three-operand
class FormatV<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  field bits<32> Inst;
  let Size = 4;

  bits<5> r2;
  bits<5> r1;
  bits<16> imm;

  let Inst{31-16} = imm;
  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

// load/store
class FormatVI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  field bits<32> Inst;
  let Size = 4;

  bits<5> r2;
  bits<5> r1;
  bits<16> disp;

  let Inst{31-16} = disp;
  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}
