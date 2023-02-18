#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "anx.h"
#include "ast.h"
#include "ir.h"

//===---------------------------------------------------------------------===//
// Anx - This module is the entry point of the Anx compiler.
//
// There are four major steps involved in the compilation process:
// 1. (lexer.cpp) Tokenize the input file
// 2. (ast.cpp) Parse the tokens into an AST
// 3. (ir.cpp) Generate LLVM IR from the AST
// 4. (anx.cpp) Generate an executable from the LLVM IR
//
// The current todo item is including more robust error messages
//===---------------------------------------------------------------------===//

std::ifstream anx::anxf;
bool anx::verbose = false;

void anx::perr(std::string msg)
{
    std::cerr << "Compilation Error:\n";
    std::cerr << "    " << msg << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    char *outfile = nullptr;

    opterr = 0;

    int c;
    while ((c = getopt(argc, argv, "vho:")) != -1)
    {
        switch (c)
        {
        case 'v':
            anx::verbose = true;
            break;
        case 'o':
            outfile = optarg;
            break;
        case '?':
            std::cerr << "Unknown option '-" << optopt << "'.\n";
            return 1;
        case 'h':
            std::cout << "USAGE: anx [options] file\n";
            std::cout << "OPTIONS:\n";
            std::cout << "  -v    Verbose mode\n";
            std::cout << "  -h    Print this help message\n";
            return 1;
        default:
            exit(1);
        }
    }

    if (argc - optind != 1)
    {
        std::cerr << "USAGE: anx [options] file\n";
        return 1;
    }

    anx::anxf.open(argv[optind]);

    if (!anx::anxf.is_open())
    {
        std::cerr << "Could not open file: " << argv[optind] << '\n';
        return 1;
    }

    ast::gen_ast();

    if (anx::verbose)
        ast::prog->print(0);

    ir::ctx = std::make_unique<llvm::LLVMContext>();
    ir::mod = std::make_unique<llvm::Module>("Anx Main", *ir::ctx);
    ir::builder = std::make_unique<llvm::IRBuilder<>>(*ir::ctx);

    ast::prog->codegen();

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    ir::mod->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        llvm::errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM, llvm::None, llvm::CodeGenOpt::Aggressive);

    ir::mod->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "out.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*ir::mod);
    dest.flush();

    // developer comment: the library location is currently hardcoded but will be eventually dealt with more formally.
    std::string linkercmd = "cc -O3 out.o /Users/tomer/Documents/GitHub/anxlang/lib/intr.c";
    if (outfile)
    {
        linkercmd += " -o";
        linkercmd += outfile;
    }

    system(linkercmd.c_str());
    remove("out.o");

    return 0;
}
