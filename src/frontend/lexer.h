#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../anx.h"

namespace lex {
enum TokEnum {
  // general
  tok_eof, // end of file
  tok_eol, // end of line

  // grouping
  tok_comma,  // ,
  tok_curlys, // curly start
  tok_curlye, // curly end
  tok_parens, // paren start
  tok_parene, // paren end

  // functions
  tok_fn,  // function
  tok_pub, // public decorator
  tok_ret, // return

  // variables
  tok_var,    // variable
  tok_colon,  // :
  tok_assign, // =

  // comparison / arithmetic
  tok_binop, // binary operation
  tok_unop,  // unary operation

  // control
  tok_if,    // if
  tok_else,  // else
  tok_while, // for
  tok_break, // break
  tok_cont,  // continue

  // identifiers
  tok_identifier, // identifier
  tok_number,     // literal number
  tok_character,  // literal ascii character
};

struct Token {
  TokEnum tok;
  std::string val;
};

extern std::vector<std::string> src;
extern Token tok;
extern anx::Pos c, l;
extern size_t ls;

void eat();
void exp(TokEnum token, std::string msg);
} // namespace lex
