#include "ABIV810.h"

#include "llvm/TargetParser/Triple.h"

#include "lldb/Core/PluginManager.h"
#include "lldb/Target/Target.h"

using namespace lldb;
using namespace lldb_private;

LLDB_PLUGIN_DEFINE(ABIV810)

enum dwarf_regnums {
  dwarf_r0 = 0,
  dwarf_r1,
  dwarf_r2,
  dwarf_r3,
  dwarf_r4,
  dwarf_r5,
  dwarf_r6,
  dwarf_r7,
  dwarf_r8,
  dwarf_r9,
  dwarf_r10,
  dwarf_r11,
  dwarf_r12,
  dwarf_r13,
  dwarf_r14,
  dwarf_r15,
  dwarf_r16,
  dwarf_r17,
  dwarf_r18,
  dwarf_r19,
  dwarf_r20,
  dwarf_r21,
  dwarf_r22,
  dwarf_r23,
  dwarf_r24,
  dwarf_r25,
  dwarf_r26,
  dwarf_r27,
  dwarf_r28,
  dwarf_r29,
  dwarf_r30,
  dwarf_r31,
  dwarf_pc
};

static const RegisterInfo g_register_infos[] = {
  {"r0", nullptr, 4, 0, eEncodingUint, eFormatHex, {dwarf_r0, dwarf_r0, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r1", nullptr, 4, 4, eEncodingUint, eFormatHex, {dwarf_r1, dwarf_r1, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"fp", "r2", 4, 8, eEncodingUint, eFormatHex, {dwarf_r2, dwarf_r2, LLDB_REGNUM_GENERIC_FP, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"sp", "r3", 4, 12, eEncodingUint, eFormatHex, {dwarf_r3, dwarf_r3, LLDB_REGNUM_GENERIC_SP, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"gp", "r4", 4, 16, eEncodingUint, eFormatHex, {dwarf_r4, dwarf_r4, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"tp", "r5", 4, 20, eEncodingUint, eFormatHex, {dwarf_r5, dwarf_r5, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r6", nullptr, 4, 24, eEncodingUint, eFormatHex, {dwarf_r6, dwarf_r6, LLDB_REGNUM_GENERIC_ARG1, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r7", nullptr, 4, 28, eEncodingUint, eFormatHex, {dwarf_r7, dwarf_r7, LLDB_REGNUM_GENERIC_ARG2, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r8", nullptr, 4, 32, eEncodingUint, eFormatHex, {dwarf_r8, dwarf_r8, LLDB_REGNUM_GENERIC_ARG3, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r9", nullptr, 4, 36, eEncodingUint, eFormatHex, {dwarf_r9, dwarf_r9, LLDB_REGNUM_GENERIC_ARG4, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r10", nullptr, 4, 40, eEncodingUint, eFormatHex, {dwarf_r10, dwarf_r10, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r11", nullptr, 4, 44, eEncodingUint, eFormatHex, {dwarf_r11, dwarf_r11, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r12", nullptr, 4, 48, eEncodingUint, eFormatHex, {dwarf_r12, dwarf_r12, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r13", nullptr, 4, 52, eEncodingUint, eFormatHex, {dwarf_r13, dwarf_r13, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r14", nullptr, 4, 56, eEncodingUint, eFormatHex, {dwarf_r14, dwarf_r14, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r15", nullptr, 4, 60, eEncodingUint, eFormatHex, {dwarf_r15, dwarf_r15, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r16", nullptr, 4, 64, eEncodingUint, eFormatHex, {dwarf_r16, dwarf_r16, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r17", nullptr, 4, 68, eEncodingUint, eFormatHex, {dwarf_r17, dwarf_r17, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r18", nullptr, 4, 72, eEncodingUint, eFormatHex, {dwarf_r18, dwarf_r18, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r19", nullptr, 4, 76, eEncodingUint, eFormatHex, {dwarf_r19, dwarf_r19, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r20", nullptr, 4, 80, eEncodingUint, eFormatHex, {dwarf_r20, dwarf_r20, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r21", nullptr, 4, 84, eEncodingUint, eFormatHex, {dwarf_r21, dwarf_r21, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r22", nullptr, 4, 88, eEncodingUint, eFormatHex, {dwarf_r22, dwarf_r22, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r23", nullptr, 4, 92, eEncodingUint, eFormatHex, {dwarf_r23, dwarf_r23, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r24", nullptr, 4, 96, eEncodingUint, eFormatHex, {dwarf_r24, dwarf_r24, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r25", nullptr, 4, 100, eEncodingUint, eFormatHex, {dwarf_r25, dwarf_r25, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r26", nullptr, 4, 104, eEncodingUint, eFormatHex, {dwarf_r26, dwarf_r26, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r27", nullptr, 4, 108, eEncodingUint, eFormatHex, {dwarf_r27, dwarf_r27, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r28", nullptr, 4, 112, eEncodingUint, eFormatHex, {dwarf_r28, dwarf_r28, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r29", nullptr, 4, 116, eEncodingUint, eFormatHex, {dwarf_r29, dwarf_r29, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r30", nullptr, 4, 120, eEncodingUint, eFormatHex, {dwarf_r30, dwarf_r30, LLDB_INVALID_REGNUM, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"r31", nullptr, 4, 124, eEncodingUint, eFormatHex, {dwarf_r31, dwarf_r31, LLDB_REGNUM_GENERIC_RA, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
  {"pc", nullptr, 4, 128, eEncodingUint, eFormatHex, {dwarf_pc, dwarf_pc, LLDB_REGNUM_GENERIC_PC, LLDB_INVALID_REGNUM}, nullptr, nullptr, nullptr},
};

static const uint32_t k_num_register_infos = std::size(g_register_infos);

const lldb_private::RegisterInfo *
ABIV810::GetRegisterInfoArray(uint32_t &count) {
  count = k_num_register_infos;
  return g_register_infos;
}

bool ABIV810::PrepareTrivialCall(lldb_private::Thread &thread, lldb::addr_t sp,
                                 lldb::addr_t functionAddress,
                                 lldb::addr_t returnAddress,
                                 llvm::ArrayRef<lldb::addr_t> args) const {
  llvm_unreachable("PrepareTrivialCall not implemented");
}

bool ABIV810::GetArgumentValues(lldb_private::Thread &thread,
                                lldb_private::ValueList &values) const {
  llvm_unreachable("GetArgumentValues not implemented");
}

lldb_private::Status
ABIV810::SetReturnValueObject(lldb::StackFrameSP &frame_sp,
                              lldb::ValueObjectSP &new_value) {
  llvm_unreachable("SetReturnValueObject not implemented");
}

lldb::ValueObjectSP
ABIV810::GetReturnValueObjectImpl(lldb_private::Thread &thread,
                                  lldb_private::CompilerType &type) const {
  llvm_unreachable("GetReturnValueObjectImpl not implemented");
}

bool
ABIV810::CreateFunctionEntryUnwindPlan(lldb_private::UnwindPlan &unwind_plan) {
  return false;
}

bool
ABIV810::CreateDefaultUnwindPlan(lldb_private::UnwindPlan &unwind_plan) {
  unwind_plan.Clear();
  unwind_plan.SetRegisterKind(eRegisterKindDWARF);

  UnwindPlan::RowSP row(new UnwindPlan::Row);

  row->GetCFAValue().SetIsRegisterDereferenced(dwarf_r2);
  row->SetRegisterLocationToAtCFAPlusOffset(dwarf_r31, 0, true);

  unwind_plan.AppendRow(row);
  unwind_plan.SetSourceName("v810 default unwind plan");
  unwind_plan.SetSourcedFromCompiler(eLazyBoolNo);
  unwind_plan.SetUnwindPlanValidAtAllInstructions(eLazyBoolNo);
  unwind_plan.SetUnwindPlanForSignalTrap(eLazyBoolNo);
  return true;
}

ABISP
ABIV810::CreateInstance(lldb::ProcessSP process_sp, const ArchSpec &arch) {
  const llvm::Triple::ArchType arch_type = arch.GetTriple().getArch();
  if (arch_type == llvm::Triple::v810) {
    return ABISP(new ABIV810(std::move(process_sp), MakeMCRegisterInfo(arch)));
  }
  return ABISP();
}

void ABIV810::Initialize() {
  PluginManager::RegisterPlugin(
    GetPluginNameStatic(), "ABI for v810 targets", CreateInstance);
}

void ABIV810::Terminate() {
  PluginManager::UnregisterPlugin(CreateInstance);
}