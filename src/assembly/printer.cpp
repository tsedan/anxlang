#include "printer.h"
#include "../codegen/ir.h"

//===---------------------------------------------------------------------===//
// Printer - This module houses the assembly and linking stages.
//===---------------------------------------------------------------------===//

void printer::init() {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
}

void printer::print() {
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  ir::mod->setTargetTriple(TargetTriple);

  std::string Error;

  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
  if (!Target)
    anx::perr(Error);

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = std::optional<llvm::Reloc::Model>();
  auto TheTargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM,
                                  std::nullopt, llvm::CodeGenOpt::Aggressive);

  ir::mod->setDataLayout(TheTargetMachine->createDataLayout());

  auto Filename = "out.o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

  if (EC)
    anx::perr("could not open file '" + EC.message() + "'");

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    anx::perr("targetmachine cannot emit a file of this type");

  pass.run(*ir::mod);
  dest.flush();
}

void printer::link(std::string filename) {
  // developer comment: the library location is currently hardcoded but will be
  // eventually dealt with more formally.
  std::string linkercmd =
      "cc -O3 out.o /Users/tomer/Documents/GitHub/anxlang/lib/intr.c -o" +
      filename;

  system(linkercmd.c_str());
}

void printer::clean() { remove("out.o"); }
