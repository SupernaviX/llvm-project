#include "MCTargetDesc/V810MCExpr.h"
#include "MCTargetDesc/V810MCTargetDesc.h"
#include "TargetInfo/V810TargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

namespace {

class V810Operand;

class V810AsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
#define GET_ASSEMBLER_HEADER
#include "V810GenAsmMatcher.inc"

  // public interface of the MCTargetAsmParser.
  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                     SMLoc &EndLoc) override;
  OperandMatchResultTy tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool ParseDirective(AsmToken DirectiveID) override;

  OperandMatchResultTy parseOperand(OperandVector &Operands, StringRef Name);

  OperandMatchResultTy parseV810AsmOperand(std::unique_ptr<V810Operand> &Operand);

public:
  V810AsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                const MCInstrInfo &MII,
                const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII), Parser(parser) {

    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));
  }
};

} // end anonymous namespace

namespace {

// V810Operand - Instances of this class represent a parsed V810 instruction.
class V810Operand : public MCParsedAsmOperand {
private:
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
    k_Memory,
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned Base;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
  };

public:
  V810Operand(KindTy K) : Kind(K) {}

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override { return Kind == k_Register; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isMem() const override { return Kind == k_Memory; }

  unsigned getReg() const override {
    assert((Kind == k_Register) && "Invalid access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  unsigned getMemBase() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Base;
  }

  const MCExpr *getMemOff() const {
    assert((Kind == k_Memory) && "Invalid access!");
    return Mem.Off;
  }

  SMLoc getStartLoc() const override {
    return StartLoc;
  }

  SMLoc getEndLoc() const override {
    return EndLoc;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case k_Token:     OS << "Token: " << getToken() << "\n"; break;
    case k_Register:  OS << "Reg: #" << getReg() << "\n"; break;
    case k_Immediate: OS << "Imm: " << getImm() << "\n"; break;
    case k_Memory:    assert(getMemOff() != nullptr);
      OS << "Mem: " << getMemBase() << "+" << getMemOff() << "\n"; break;
    }
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const{
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  static std::unique_ptr<V810Operand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<V810Operand>(k_Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<V810Operand> CreateReg(unsigned RegNum, SMLoc S,
                                                SMLoc E) {
    auto Op = std::make_unique<V810Operand>(k_Register);
    Op->Reg.RegNum = RegNum;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<V810Operand> CreateImm(const MCExpr *Val, SMLoc S,
                                                SMLoc E) {
    auto Op = std::make_unique<V810Operand>(k_Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

};

}

/// Maps from the set of all register names to a register number.
/// \note Generated by TableGen.
static unsigned MatchRegisterName(StringRef Name);
static unsigned MatchRegisterAltName(StringRef Alias);

bool V810AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (MatchResult) {
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MissingFeature:
    return Error(IDLoc,
                 "instruction requires a CPU feature not currently enabled");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");
      ErrorLoc = ((V810Operand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "invalid instruction mnemonic");
  }
  llvm_unreachable("Implement any new match types added!");
}

bool V810AsmParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                   SMLoc &EndLoc) {
  if (tryParseRegister(RegNo, StartLoc, EndLoc) != MatchOperand_Success)
    return Error(StartLoc, "invalid register name");
  return false;
}

OperandMatchResultTy V810AsmParser::tryParseRegister(MCRegister &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  const AsmToken Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  RegNo = MatchRegisterName(Tok.getString());
  if (RegNo) {
    Parser.Lex(); // eat the identifier
    return MatchOperand_Success;
  }
  RegNo = MatchRegisterAltName(Tok.getString());
  if (RegNo) {
    Parser.Lex();
    return MatchOperand_Success;
  }
  return MatchOperand_NoMatch;
}

bool V810AsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                     StringRef Name, SMLoc NameLoc,
                                     OperandVector &Operands) {
  // TODO: bcond and setf make this part weird
  Operands.push_back(V810Operand::CreateToken(Name, NameLoc));

  // read first operand  
  if (parseOperand(Operands, Name) != MatchOperand_Success) {
    SMLoc Loc = getLexer().getLoc();
    return Error(Loc, "unexpected token");
  }

  // read other operands
  while (getLexer().is(AsmToken::Comma)) {
    getLexer().Lex();
    if (parseOperand(Operands, Name) != MatchOperand_Success) {
      SMLoc Loc = getLexer().getLoc();
      return Error(Loc, "unexpected token");
    }
  }

  return getLexer().isNot(AsmToken::EndOfStatement);
}

bool V810AsmParser::ParseDirective(AsmToken DirectiveID) {
  // Let the MC layer handle everything
  return true;
}

OperandMatchResultTy
V810AsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  std::unique_ptr<V810Operand> Op;
  OperandMatchResultTy ResTy = parseV810AsmOperand(Op);
  if (ResTy != MatchOperand_Success || !Op)
    return MatchOperand_ParseFail;
  
  Operands.push_back(std::move(Op));

  return MatchOperand_Success;
}

OperandMatchResultTy
V810AsmParser::parseV810AsmOperand(std::unique_ptr<V810Operand> &Op) {
  SMLoc Start = Parser.getTok().getLoc();
  SMLoc End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  const MCExpr *EVal;

  Op = nullptr;
  switch (getLexer().getKind()) {
  default:  break;

  case AsmToken::Plus:
  case AsmToken::Minus:
  case AsmToken::Integer:
    if (getParser().parseExpression(EVal, End))
      break;
    int64_t Res;
    if (!EVal->evaluateAsAbsolute(Res)) {
      // TODO: variant kind
      V810MCExpr::VariantKind Kind = V810MCExpr::VK_V810_None;
      EVal = V810MCExpr::create(Kind, EVal, getContext());
    }
    Op = V810Operand::CreateImm(EVal, Start, End);
    break;
  case AsmToken::Identifier:
    MCRegister IndexReg;
    OperandMatchResultTy result = tryParseRegister(IndexReg, Start, End);
    if (result == MatchOperand_Success) {
      Op = V810Operand::CreateReg(IndexReg, Start, End);
      break;
    }
  }

  return (Op) ? MatchOperand_Success : MatchOperand_ParseFail;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeV810AsmParser() {
  RegisterMCAsmParser<V810AsmParser> X(getTheV810Target());
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "V810GenAsmMatcher.inc"