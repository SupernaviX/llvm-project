#include "V810InstPrinter.h"
#include "V810.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "V810GenAsmWriter.inc"

void V810InstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) const {
  OS << StringRef(getRegisterName(Reg));
}

void V810InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O)) {
    if (MI->getOpcode() == V810::Bcond)
      printBcondInstruction(MI, Address, O);
    else if (MI->getOpcode() == V810::JMP)
      printJmpInstruction(MI, Address, O);
    else
      printInstruction(MI, Address, O);
  }
  printAnnotation(O, Annot);
}

void V810InstPrinter::printBcondInstruction(const MCInst *MI, uint64_t Address,
                                            raw_ostream &O) {
  assert(MI->getNumOperands() == 2);
  int64_t cond = MI->getOperand(0).getImm();

  if (cond == V810CC::ICC_NOP) {
    O << "\tnop";
  } else {
    printInstruction(MI, Address, O);
  }
}

void V810InstPrinter::printJmpInstruction(const MCInst *MI, uint64_t Address,
                                          raw_ostream &O) {
  O <<"\tjmp [";
  printOperand(MI, 0, O);
  O << "]";
}

void V810InstPrinter::printOperand(const MCInst *MI, int opNum,
                                   raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << formatImm(MO.getImm());
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void V810InstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                         unsigned opNum, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm()) {
    int64_t Val = MO.getImm();
    if (Val > 0) O << '+';
    O << formatImm(Val);
    return;
  }

  assert(MO.isExpr() && "not expr?");
  MO.getExpr()->print(O, &MAI);
}

void V810InstPrinter::printMemOperand(const MCInst *MI, int opNum,
                                      raw_ostream &O) {
  printOperand(MI, opNum + 1, O);
  O << "[";
  printOperand(MI, opNum, O);
  O << "]";
}

void V810InstPrinter::printCondOperand(const MCInst *MI, int opNum,
                                       raw_ostream &O) {
  V810CC::CondCodes Cond = (V810CC::CondCodes)MI->getOperand(opNum).getImm();
  O << V810CondCodeToString(Cond);
}