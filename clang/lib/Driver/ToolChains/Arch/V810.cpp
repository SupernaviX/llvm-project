#include "V810.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"
#include "llvm/TargetParser/Host.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

std::string v810::getV810TargetCPU(const Driver &D, const ArgList &Args,
                                   const llvm::Triple &Triple) {

  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_march_EQ)) {
    D.Diag(diag::err_drv_unsupported_opt_for_target)
        << A->getSpelling() << Triple.getTriple();
    return "";
  }

  if (const Arg *A = Args.getLastArg(clang::driver::options::OPT_mcpu_EQ)) {
    StringRef CPUName = A->getValue();
    return std::string(CPUName);
  }

  return "";

}