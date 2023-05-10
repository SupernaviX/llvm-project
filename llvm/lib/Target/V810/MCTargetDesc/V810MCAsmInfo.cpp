#include "V810MCAsmInfo.h"

using namespace llvm;

void V810AsmInfo::anchor() {}

V810AsmInfo::V810AsmInfo(const Triple &TheTriple) {
  IsLittleEndian = true;
}