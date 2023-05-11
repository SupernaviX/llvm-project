add_llvm_component_library(LLVMV810Desc
  V810AsmBackend.cpp
  V810MCAsmInfo.cpp
  V810MCCodeEmitter.cpp
  V810MCTargetDesc.cpp
  V810ObjectWriter.cpp

  LINK_COMPONENTS
  MC
  V810Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  V810
  )