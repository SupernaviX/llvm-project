include "llvm/Target/Target.td"

include "V810RegisterInfo.td"
include "V810CallingConv.td"
include "V810InstrInfo.td"

def V810InstrInfo : InstrInfo;

def V810AsmParser : AsmParser;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def V810AsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstrPrinter";
}

def V810 : Target {
  let InstructionSet = V810InstrInfo;
  let AssemblyParsers = [V810AsmParser];
  let AssemblyWriters = [V810AsmWriter];
}