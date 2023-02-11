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
// 1. (lexer.cpp) Tokenize the input file - mvp complete
// 2. (ast.cpp) Parse the tokens into an AST - mvp complete
// 3. (ir.cpp) Generate LLVM IR from the AST - mvp complete
// 4. (anx.cpp) Generate an executable from the LLVM IR - mvp complete
//
// The current todo item is implementing variables.
//===---------------------------------------------------------------------===//

std::ifstream anxf;
bool verbose = false;

void perr(std::string msg)
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
            verbose = true;
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
            abort();
        }
    }

    if (argc - optind != 1)
    {
        std::cerr << "USAGE: anx [options] file\n";
        return 1;
    }

    anxf.open(argv[optind]);

    if (!anxf.is_open())
    {
        std::cerr << "Could not open file: " << argv[optind] << '\n';
        return 1;
    }

    std::unique_ptr<ast::ProgramNode> program = ast::gen_ast();

    if (verbose)
        program->print(0);

    ir::ctx = std::make_unique<llvm::LLVMContext>();
    ir::mod = std::make_unique<llvm::Module>("Anx Main", *ir::ctx);
    ir::builder = std::make_unique<llvm::IRBuilder<>>(*ir::ctx);

    program->codegen();

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

    std::string linkercmd = "cc out.o";
    if (outfile)
    {
        linkercmd += " -o";
        linkercmd += outfile;
    }
    if (verbose)
        linkercmd += " -v";

    system(linkercmd.c_str());
    remove("out.o");

    return 0;
}
