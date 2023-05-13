class V810Reg<bits<16> Enc, string n, list<string> altNames = []> : Register<n, altNames> {
  let HWEncoding = Enc;
  let Namespace = "V810";
}

def R0  : V810Reg< 0, "r0">, DwarfRegNum<[0]> {
  let isConstant = true;
}
def R1  : V810Reg< 1, "r1">, DwarfRegNum<[1]>;
def R2  : V810Reg< 2, "r2", ["hp"]>, DwarfRegNum<[2]>;
def R3  : V810Reg< 3, "r3", ["sp"]>, DwarfRegNum<[3]>;
def R4  : V810Reg< 4, "r4", ["gp"]>, DwarfRegNum<[4]>;
def R5  : V810Reg< 5, "r5", ["tp"]>, DwarfRegNum<[5]>;
def R6  : V810Reg< 6, "r6">, DwarfRegNum<[6]>;
def R7  : V810Reg< 7, "r7">, DwarfRegNum<[7]>;
def R8  : V810Reg< 8, "r8">, DwarfRegNum<[8]>;
def R9  : V810Reg< 9, "r9">, DwarfRegNum<[9]>;
def R10 : V810Reg<10, "r10">, DwarfRegNum<[10]>;
def R31 : V810Reg<31, "r31", ["lp"]>, DwarfRegNum<[31]>;

def GenRegs : RegisterClass<"V810", [i32], 32, (add (sequence "R%u", 0, 10), R31)>;