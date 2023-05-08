class V810Reg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "V810";
}

def R0 : V810Reg<0, "R0">, DwarfRegNum<[0]> {
  let isConstant = true;
}
def R1 : V810Reg<1, "R1">, DwarfRegNum<[1]>;

def GenRegs : RegisterClass<"V810", [i32], 32, (sequence "R%u", 0, 1)>;