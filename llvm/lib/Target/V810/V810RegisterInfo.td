class V810Reg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "V810";
}

def R0  : V810Reg< 0, "r0" >, DwarfRegNum<[0]> {
  let isConstant = true;
}
def R1  : V810Reg< 1, "r1" >, DwarfRegNum<[1]>;
def R2  : V810Reg< 2, "hp" >, DwarfRegNum<[2]>;
def R3  : V810Reg< 3, "sp" >, DwarfRegNum<[3]>;
def R31 : V810Reg<31, "r31">, DwarfRegNum<[31]>;

def GenRegs : RegisterClass<"V810", [i32], 32, (add R0, R1, R2, R3, R31)>;