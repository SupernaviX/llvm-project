#ifndef LLVM_LIB_TARGET_V810_V810TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_V810_V810TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class MCContext;
class TargetMachine;

class V810TargetObjectFile : public TargetLoweringObjectFileELF {
public:
  V810TargetObjectFile() = default;
};

} // end namespace llvm

#endif