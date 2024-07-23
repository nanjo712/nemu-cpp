/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#if LLVM_VERSION_MAJOR >= 14
#include "llvm/MC/TargetRegistry.h"
#if LLVM_VERSION_MAJOR >= 15
#include "llvm/MC/MCSubtargetInfo.h"
#endif
#else
#include "llvm/Support/TargetRegistry.h"
#endif
#include "llvm/Support/TargetSelect.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#if LLVM_VERSION_MAJOR < 11
#error Please use LLVM with major version >= 11
#endif

using namespace llvm;

static llvm::MCDisassembler *gDisassembler = nullptr;
static llvm::MCSubtargetInfo *gSTI = nullptr;
static llvm::MCInstPrinter *gIP = nullptr;

void init_disasm(const char *triple)
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllDisassemblers();

    std::string errstr;
    std::string gTriple(triple);

    llvm::MCInstrInfo *gMII = nullptr;
    llvm::MCRegisterInfo *gMRI = nullptr;
    auto target = llvm::TargetRegistry::lookupTarget(gTriple, errstr);
    if (!target)
    {
        llvm::errs() << "Can't find target for " << gTriple << ": " << errstr
                     << "\n";
        assert(0);
    }

    MCTargetOptions MCOptions;
    gSTI = target->createMCSubtargetInfo(gTriple, "", "");
    std::string isa = target->getName();
    if (isa == "riscv32" || isa == "riscv64")
    {
        gSTI->ApplyFeatureFlag("+m");
        gSTI->ApplyFeatureFlag("+a");
        gSTI->ApplyFeatureFlag("+c");
        gSTI->ApplyFeatureFlag("+f");
        gSTI->ApplyFeatureFlag("+d");
    }
    gMII = target->createMCInstrInfo();
    gMRI = target->createMCRegInfo(gTriple);
    auto AsmInfo = target->createMCAsmInfo(*gMRI, gTriple, MCOptions);
#if LLVM_VERSION_MAJOR >= 13
    auto llvmTripleTwine = Twine(triple);
    auto llvmtriple = llvm::Triple(llvmTripleTwine);
    auto Ctx = new llvm::MCContext(llvmtriple, AsmInfo, gMRI, nullptr);
#else
    auto Ctx = new llvm::MCContext(AsmInfo, gMRI, nullptr);
#endif
    gDisassembler = target->createMCDisassembler(*gSTI, *Ctx);
    gIP = target->createMCInstPrinter(llvm::Triple(gTriple),
                                      AsmInfo->getAssemblerDialect(), *AsmInfo,
                                      *gMII, *gMRI);
    gIP->setPrintImmHex(true);
    gIP->setPrintBranchImmAsAddress(true);
    if (isa == "riscv32" || isa == "riscv64")
        gIP->applyTargetSpecificCLOption("no-aliases");
}

std::string disassemble(uint64_t pc, uint8_t *code, int nbyte)
{
    // char *p = s->logbuf;
    // p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
    // int ilen = s->snpc - s->pc;
    // int i;
    // uint8_t *inst = (uint8_t *)&s->isa.inst.val;
    // for (i = ilen - 1; i >= 0; i--)
    // {
    //     p += snprintf(p, 4, " %02x", inst[i]);
    // }
    // int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    // int space_len = ilen_max - ilen;
    // if (space_len < 0) space_len = 0;
    // space_len = space_len * 3 + 1;
    // memset(p, ' ', space_len);
    // p += space_len;
    std::string ret;
    raw_string_ostream os(ret);
    os << format_hex(pc, 2) << ":";
    for (int i = nbyte - 1; i >= 0; i--)
    {
        os << format(" %02x", code[i]);
    }
    int ilen_max = 4;
    int space_len = ilen_max - nbyte;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;
    os << std::string(space_len, ' ');

    MCInst inst;
    llvm::ArrayRef<uint8_t> arr(code, nbyte);
    uint64_t dummy_size = 0;
    gDisassembler->getInstruction(inst, dummy_size, arr, pc, llvm::nulls());

    gIP->printInst(&inst, pc, "", *gSTI, os);

    return os.str();
}
