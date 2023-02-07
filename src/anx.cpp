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
// 2. (ast.cpp) Parse the tokens into an AST - in progress
// 3. (ir.cpp) Generate LLVM IR from the AST - not started
// 4. Generate an executable from the LLVM IR - not started
//===---------------------------------------------------------------------===//

std::ifstream anxf;

void perr(std::string msg)
{
    std::cerr << "Compilation Error:\n";
    std::cerr << "    " << msg << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    bool verbose = false;

    opterr = 0;

    int c;
    while ((c = getopt(argc, argv, "vh")) != -1)
    {
        switch (c)
        {
        case 'v':
            verbose = true;
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
}
