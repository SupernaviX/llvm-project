#include "V810HazardRecognizer.h"
#include "V810.h"
#include "llvm/CodeGen/ScheduleDAG.h"

using namespace llvm;

static bool isStore(const MachineInstr *MI) {
  // TODO: is there a better way to do this than hard-coding registers?
  switch (MI->getOpcode()) {
  case V810::ST_W:
  case V810::OUT_W:
  case V810::ST_H:
  case V810::OUT_H:
  case V810::ST_B:
  case V810::OUT_B:
    return true;
  default:
    return false;
  }
}

ScheduleHazardRecognizer::HazardType
V810HazardRecognizer::getHazardType(SUnit *SU, int Stalls) {
  // Assume the first instruction in a block can't stall
  if (!LastSU) {
    return NoHazard;
  }

  // If there was nothing we could schedule to avoid a stall,
  // assume the stall happened and this next instr is no longer a hazard.
  if (CyclesSinceLastEmit > LastSU->getHeight() + 1u) {
    return NoHazard;
  }

  if (isStore(SU->getInstr()) && isStore(LastSU->getInstr())) {
    return Hazard;
  }
  
  return NoHazard;
}

bool V810HazardRecognizer::ShouldPreferAnother(SUnit *SU) {
  // Schedule ASAP anything which COULD trigger a hazard later,
  // so the scheduler is more free to move instrs between the first and second
  return !isStore(SU->getInstr());
}

void V810HazardRecognizer::Reset() {
  LastSU = nullptr;
  CyclesSinceLastEmit = 0;
}

void V810HazardRecognizer::EmitInstruction(SUnit *SU) {
  LastSU = SU;
  CyclesSinceLastEmit = 0;
}

void V810HazardRecognizer::AdvanceCycle() {
  CyclesSinceLastEmit++;
}

void V810HazardRecognizer::RecedeCycle() {
  llvm_unreachable("Scheduling works top-down");
}