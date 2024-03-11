
// The Schedule Machine Model itself

def V810Model : SchedMachineModel {
  let MicroOpBufferSize = 0; // In-order
  let IssueWidth = 1; // One thing happens per cycle
  let PostRAScheduler = 1;

}

// Resources used by scheduler definitions
// (the 1 means there's only one of each)
let BufferSize = 0 in {
def V810UnitMemory  : ProcResource<1>;
def V810UnitBranch  : ProcResource<1>;
def V810UnitFPU     : ProcResource<1>;
}

// Scheduler Definitions
// (the result/operands of different instructions use these)
def WriteI        : SchedWrite; // math-free MOVs
def WriteALU      : SchedWrite; // most integer math can be treated the same
def WriteIMul     : SchedWrite; // MUL/MULU
def WriteIDiv     : SchedWrite; // DIV/DIVU
def WriteFPU      : SchedWrite; // being lazy and treating FP math the same too
def WriteLoad     : SchedWrite; // Load from memory
def WriteStore    : SchedWrite; // Store to memory
def WriteBranch   : SchedWrite; // jump jump

// Tie it all together
let SchedModel = V810Model in {

def : WriteRes<WriteI, []> { let Latency = 1; }
def : WriteRes<WriteALU, []> { let Latency = 1; }
def : WriteRes<WriteIMul, []> { let Latency = 13; }
def : WriteRes<WriteIDiv, []> { let Latency = 38; }
def : WriteRes<WriteFPU, [V810UnitFPU]> { let Latency = 28; } // everything is the same number of cycles because I am lazy
def : WriteRes<WriteLoad, [V810UnitMemory]> { let Latency = 1; } // TODO: translate timings to scheduler model 
def : WriteRes<WriteStore, [V810UnitMemory]>  { let Latency = 1; } // TODO: translate timings to scheduler model
def : WriteRes<WriteBranch, [V810UnitBranch]> { let Latency = 3; } // TODO: how do I say "this flushes the pipeline"

// the COPY pseudo-instr needs scheduling info
def : InstRW<[WriteI], (instrs COPY)>;

}