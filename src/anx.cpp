#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

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

std::vector<std::string> anx::file;
bool anx::verbose = false;
std::string src;

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
            anx::perr(std::string("unknown compiler option '-") + (char)optopt + "' (use -h for help)");
        case 'h':
            std::cerr << "USAGE: anx [options] file\n";
            std::cerr << "OPTIONS:\n";
            std::cerr << "  -v    Verbose mode\n";
            std::cerr << "  -h    Print this help message\n";
        default:
            exit(1);
        }
    }

    if (argc - optind != 1)
        anx::perr("usage: anx [options] file (use -h for help)");

    src = argv[optind];

    std::ifstream anxf(src);
    if (!anxf.is_open())
        anx::perr("could not open file '" + src + "'");

    for (std::string line; std::getline(anxf, line);)
        anx::file.push_back(line);

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
        anx::perr(Error);

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
        anx::perr("could not open file '" + EC.message() + "'");

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        anx::perr("targetmachine cannot emit a file of this type");

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

void anx::perr(std::string msg, size_t r, size_t c, size_t s)
{
    std::string line = file[r];
    size_t p = c, begin = line.find_first_not_of(" \t"), end = line.find_last_not_of(" \t");
    if (begin != std::string::npos)
    {
        line = line.substr(begin, end - begin + 1);
        p -= begin;
    }

    size_t len = std::max(line.size(), p + s);
    std::string ep0(c - p, ' ');
    std::string ep1(p, '~');
    std::string ep2(s, '^');
    std::string ep3(len - s - p, '~');

    std::cerr << "\033[0;31merror: \033[0m" << msg << '\n';

    std::cerr << "  --> " << src << ':' << r + 1 << ':' << c + 1;
    if (s > 1)
        std::cerr << '-' << c + s;
    std::cerr << '\n';

    std::cerr << "   | " << ep0 << line << '\n';
    std::cerr << "   | " << ep0 << ep1 << "\033[0;31m" << ep2 << "\033[0m" << ep3 << '\n';

    exit(1);
}

void anx::perr(std::string msg)
{
    std::cerr << "\033[0;31merror: \033[0m" << msg << '\n';
    exit(1);
}
