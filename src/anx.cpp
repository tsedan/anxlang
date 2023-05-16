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
// utils.cpp holds a few miscellaneous long functions used in these steps.
//
// The current todo item is increment / decrement.
//===---------------------------------------------------------------------===//

std::string src;

int main(int argc, char **argv) {
  std::string outfile = "a.out";

  opterr = 0;

  int c;
  while ((c = getopt(argc, argv, "ho:")) != -1) {
    switch (c) {
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

  if (argc - optind != 1) {
    // shell JIT mode

    anx::perr("this feature is still under development.");
  } else {
    // compiler mode

    src = argv[optind];

    lex::read(src);

    auto prog = ast::generate();

    ir::init(src);

    prog->codegen();

    printer::init();
    printer::print();
    printer::link(outfile);
    printer::clean();
  }

  return 0;
}

void anx::perr(std::string msg, Pos pos, size_t size) {
  std::string line = lex::src[pos.r], ep0;
  size_t p = pos.c, begin = line.find_first_not_of(" \t"),
         end = line.find_last_not_of(" \t");
  if (begin != std::string::npos) {
    ep0 = line.substr(0, begin);
    line = line.substr(begin, end - begin + 1);
    if (p > begin)
      p -= begin;
    else
      p = 0;
  }

  size_t len = std::max(line.size(), p + size);

  std::string ep1(p, '~'), ep2(size, '^'), ep3(len - size - p, '~');

  std::cerr << "\033[0;31merror: \033[0m" << msg << '\n';

  std::cerr << "  --> " << src << ':' << pos.r + 1 << ':' << pos.c + 1;
  if (size > 1)
    std::cerr << '-' << pos.c + size;
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
