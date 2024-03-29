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
// 0. (anx.cpp) (this file) Orchestrate compilation / JIT processes
// 1. (frontend/lexer.cpp) Tokenize source code input
// 2. (frontend/ast.cpp) Parse AST from tokens
// 3. (codegen/ir.cpp) Generate LLVM IR from AST
// 4. (codegen/opti.cpp) Optimize LLVM IR
// 5. (assembly/printer.cpp) Assemble executable from IR
// utils.cpp holds a few misc. large functions used in these steps.
//
//===---------------------------------------------------------------------===//

std::istream *anx::stream = nullptr;
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

    anx::stream = &std::cin;

    lex::eat(); // generate the first token
    auto fn = ast::step();

  } else {
    // compiler mode

    src = argv[optind];

    std::ifstream f(src);
    if (!f.is_open())
      anx::perr("could not open file '" + src + "'");

    anx::stream = &f;

    lex::eat(); // generate the first token
    auto prog = ast::unit();

    ir::init();

    prog->codegen();

    printer::init();
    printer::print();
    printer::link(outfile);
    printer::clean();
  }

  return 0;
}

void anx::perr(std::string msg, Pos pos, size_t size) {
  std::string line = lex::src[pos.r - 1], ep0;
  size_t p = pos.c, begin = line.find_first_not_of(" \t\n"),
         end = line.find_last_not_of(" \t\n");
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

  std::cerr << "  --> " << src << ':' << pos.r << ':' << pos.c + 1;
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
