include "llvm/Target/Target.td"

def FeatureNintendo
  : SubtargetFeature<"nintendo", "IsNintendo", "true",
                     "Enable Nintendo instsructions">;

include "V810RegisterInfo.td"
include "V810CallingConv.td"
include "V810Schedule.td"
include "V810InstrInfo.td"

def V810InstrInfo : InstrInfo;

def V810AsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
}

class Proc<string Name, list<SubtargetFeature> Features>
 : ProcessorModel<Name, V810Model, Features>;

def : Proc<"generic", []>;
def : Proc<"vb", [FeatureNintendo]>;

def V810AsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}

def V810 : Target {
  let InstructionSet = V810InstrInfo;
  let AssemblyParsers = [V810AsmParser];
  let AssemblyWriters = [V810AsmWriter];
}