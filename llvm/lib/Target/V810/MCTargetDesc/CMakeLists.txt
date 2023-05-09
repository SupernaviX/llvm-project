add_llvm_component_library(LLVMV810Desc
  V810MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  V810Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  V810
  )