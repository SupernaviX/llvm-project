class InstV810<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
  field bits<16> Inst;

  let Namespace = "V810";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "V810";
}

class FormatII<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  let Size = 2;

  bits<5> r2;
  bits<5> r1;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
}

class FormatVI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstV810<outs, ins, asmstr, pattern> {
  let Size = 4;

  field bits<16> Inst2;
  bits<5> r2;
  bits<5> r1;
  bits<16> disp;

  let Inst{15-10} = op;
  let Inst{9-5} = r2;
  let Inst{4-0} = r1;
  let Inst2{15-0} = disp;
}
