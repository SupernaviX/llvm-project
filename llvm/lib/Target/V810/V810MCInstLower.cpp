#include "V810.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

static MCOperand LowerGlobalOperand(const MachineInstr *MI,
                                    const MachineOperand &MO,
                                    AsmPrinter &AP) {
  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(AP.getSymbol(MO.getGlobal()),
                                                         AP.OutContext);
  return MCOperand::createExpr(MCSym);
}

static MCOperand LowerOperand(const MachineInstr *MI,
                              const MachineOperand &MO,
                              AsmPrinter &AP) {
  switch (MO.getType()) {
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_GlobalAddress:
    return LowerGlobalOperand(MI, MO, AP);
  case MachineOperand::MO_RegisterMask:
    break;
  }

  return MCOperand();
}

void llvm::LowerV810MachineInstrToMCInst(const MachineInstr *MI,
                                         MCInst &OutMI,
                                         AsmPrinter &AP)
{
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = LowerOperand(MI, MO, AP);

    if (MCOp.isValid()) {
      OutMI.addOperand(MCOp);
    }
  }
}