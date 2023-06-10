#include "V810.h"
#include "V810InstrInfo.h"
#include "V810Subtarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/MathExtras.h"
using namespace llvm;

#define DEBUG_TYPE "v810-branch-select"

namespace {
  struct V810BSel : public MachineFunctionPass {
    static char ID;
    V810BSel() : MachineFunctionPass(ID) {
      initializeV810BSelPass(*PassRegistry::getPassRegistry());
    }

    // The offsets of the basic blocks in the function.
    std::vector<unsigned> BlockOffsets;

    unsigned ComputeBlockOffsets(MachineFunction &Fn);
    int computeBranchSize(MachineFunction &Fn,
                          const MachineBasicBlock *Src,
                          const MachineBasicBlock *Dest,
                          unsigned BrOffset);

    bool runOnMachineFunction(MachineFunction &Fn) override;

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().set(
          MachineFunctionProperties::Property::NoVRegs);
    }

    StringRef getPassName() const override { return "V810 Branch Selector"; }
  };
  char V810BSel::ID = 0;
}

INITIALIZE_PASS(V810BSel, "v810-branch-select", "V810 Branch Selector",
                false, false);

FunctionPass *llvm::createV810BranchSelectionPass() {
  return new V810BSel();
}

/// Measure each MBB and compute a size for the entire function
unsigned V810BSel::ComputeBlockOffsets(MachineFunction &Fn) {
  const V810InstrInfo *TII =
    static_cast<const V810InstrInfo *>(Fn.getSubtarget().getInstrInfo());
  unsigned FuncSize = 0;

  for (MachineBasicBlock &MBB : Fn) {
    BlockOffsets[MBB.getNumber()] = FuncSize;
    for (MachineInstr &MI : MBB) {
      FuncSize += TII->getInstSizeInBytes(MI);
    }
  }
  
  return FuncSize;
}

/// Determine the offset from the branch in Src block to the Dest block.
/// BrOffset is the offset of the branch instruction inside Src block.
int V810BSel::computeBranchSize(MachineFunction &Fn,
                                const MachineBasicBlock *Src,
                                const MachineBasicBlock *Dest,
                                unsigned BrOffset) {
  int SrcAddr = BlockOffsets[Src->getNumber()] + BrOffset;
  int DestAddr = BlockOffsets[Dest->getNumber()];

  return DestAddr - SrcAddr;
}

bool V810BSel::runOnMachineFunction(MachineFunction &Fn) {
  const V810InstrInfo *TII =
    static_cast<const V810InstrInfo *>(Fn.getSubtarget().getInstrInfo());
  // Give the blocks of the function a dense, in-order, numbering.
  Fn.RenumberBlocks();
  BlockOffsets.resize(Fn.getNumBlockIDs());

  // Measure each MBB and compute a size for the entire function.
  unsigned FuncSize = ComputeBlockOffsets(Fn);

  // If the entire function is smaller than the displacement of a branch field,
  // we know we don't need to shrink any branches in this function.  This is a
  // common case.
  if (isInt<9>(FuncSize)) {
    BlockOffsets.clear();
    return false;
  }

  bool MadeChange = true;
  bool EverMadeChange = false;
  while (MadeChange) {
    // Iteratively expand branches until we reach a fixed point
    MadeChange = false;

    for (MachineFunction::iterator MFI = Fn.begin(), E = Fn.end(); MFI != E; ++MFI) {
      MachineBasicBlock &MBB = *MFI;
      unsigned MBBStartOffset = 0;
      for (MachineBasicBlock::iterator I = MBB.begin(), E = MBB.end(); I != E; ++I) {
        MachineBasicBlock *Dest = nullptr;
        MachineInstr &MI = *I;
        if (MI.getOpcode() == V810::Bcond && !MI.getOperand(1).isImm()) {
          Dest = MI.getOperand(1).getMBB();
        }

        if (!Dest) {
          MBBStartOffset += TII->getInstSizeInBytes(MI);
          continue;
        }

        int BranchSize = computeBranchSize(Fn, &MBB, Dest, MBBStartOffset);
        if (isInt<9>(BranchSize)) {
          // it fits, we're fine
          MBBStartOffset += TII->getInstSizeInBytes(MI);
          continue;
        }

        // This instruction can't reach that far, replace it with one that can
        DebugLoc dl = MI.getDebugLoc();
        int BlockSizeChange = 0;

        V810CC::CondCodes Cond = (V810CC::CondCodes) MI.getOperand(0).getImm();
        V810CC::CondCodes InvertedCond = InvertV810CondCode(Cond);

        if (InvertedCond != V810CC::CC_NOP) {
          // conditionally branch over the unconditional jump
          BuildMI(MBB, I, dl, TII->get(V810::Bcond))
            .addImm(InvertedCond)
            .addImm(6);
          BlockSizeChange += 2;          
        }

        // Unconditionally jump to where we were gonna go before
        I = BuildMI(MBB, I, dl, TII->get(V810::JR)).addMBB(Dest);
        BlockSizeChange += 4;
        
        // Remember that we're tracking our current offset in this function.
        // Add the size of the new code to that.
        MBBStartOffset += BlockSizeChange;

        // Clear the old conditional branch out, we've replaced it
        MI.eraseFromParent();
        BlockSizeChange -= 2;

        // Update the offsets of everything that comes after this block
        for (unsigned i = MBB.getNumber() + 1; i < BlockOffsets.size(); ++i) {
          BlockOffsets[i] += BlockSizeChange;
        }

        MadeChange = true;
      }
    }
    
    EverMadeChange |= MadeChange;
  }

  BlockOffsets.clear();
  return EverMadeChange;
}