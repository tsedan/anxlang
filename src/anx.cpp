#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "anx.h"
#include "assembly/printer.h"
#include "codegen/ir.h"
#include "frontend/ast.h"
#include "frontend/lexer.h"

//===---------------------------------------------------------------------===//
// Anx - This module is the entry point of the Anx compiler.
//
// These are the major steps involved in the compilation process:
// 1. (frontend/lexer.cpp) Tokenize the input file
// 2. (frontend/ast.cpp) Parse the tokens into an AST
// 3. (codegen/ir.cpp) Generate LLVM IR from the AST
// 4. (codegen/opti.cpp) Run optimization passes on the LLVM IR
// 5. (assembly/printer.cpp) Generate an executable from the LLVM IR
//
// The current todo item is loops.
//===---------------------------------------------------------------------===//

bool anx::verbose = false;
std::string src;

int main(int argc, char **argv) {
  char *outfile = nullptr;

  opterr = 0;

  int c;
  while ((c = getopt(argc, argv, "vho:")) != -1) {
    switch (c) {
    case 'v':
      anx::verbose = true;
      break;
    case 'o':
      outfile = optarg;
      break;
    case '?':
      anx::perr(std::string("unknown compiler option '-") + (char)optopt +
                "' (use -h for help)");
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
  lex::read(src);

  ast::gen_ast();

  if (anx::verbose)
    ast::prog->print(0);

  ir::init();

  ast::prog->codegen();

  printer::print();
  printer::link(outfile);
  printer::clean();

  return 0;
}

void anx::perr(std::string msg, size_t r, size_t c, size_t s) {
  std::string line = lex::file[r], ep0;
  size_t p = c, begin = line.find_first_not_of(" \t"),
         end = line.find_last_not_of(" \t");
  if (begin != std::string::npos) {
    ep0 = line.substr(0, begin);
    line = line.substr(begin, end - begin + 1);
    if (p > begin)
      p -= begin;
    else
      p = 0;
  }

  size_t len = std::max(line.size(), p + s);

  std::string ep1(p, '~'), ep2(s, '^'), ep3(len - s - p, '~');

  std::cerr << "\033[0;31merror: \033[0m" << msg << '\n';

  std::cerr << "  --> " << src << ':' << r + 1 << ':' << c + 1;
  if (s > 1)
    std::cerr << '-' << c + s;
  std::cerr << '\n';

  std::cerr << "   | " << ep0 << line << '\n';
  std::cerr << "   | " << ep0 << ep1 << "\033[0;31m" << ep2 << "\033[0m" << ep3
            << '\n';

  exit(1);
}

void anx::perr(std::string msg) {
  std::cerr << "\033[0;31merror: \033[0m" << msg << '\n';
  exit(1);
}
