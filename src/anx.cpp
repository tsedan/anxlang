#include "anx.h"
#include "lexer.h"
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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: anx <filename>\n");
        return 1;
    }

    anxf.open(argv[1]);

    if (!anxf.is_open())
    {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    while (next_token() != tok_eof)
        printf("%d ", token);
}
