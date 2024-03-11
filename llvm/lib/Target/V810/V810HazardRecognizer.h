#ifndef LLVM_LIB_TARGET_V810_V810HAZARDRECOGNIZER_H
#define LLVM_LIB_TARGET_V810_V810HAZARDRECOGNIZER_H

#include "V810InstrInfo.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"

namespace llvm {

class V810HazardRecognizer : public ScheduleHazardRecognizer {
private:
  SUnit *LastSU;
  unsigned CyclesSinceLastEmit;

public:
  V810HazardRecognizer(): LastSU(nullptr), CyclesSinceLastEmit(0) { MaxLookAhead = 1; }

  HazardType getHazardType(SUnit *SU, int Stalls) override;
  bool ShouldPreferAnother(SUnit *SU) override;
  void Reset() override;
  void EmitInstruction(SUnit *SU) override;
  void AdvanceCycle() override;
  void RecedeCycle() override;
};

}

#endif