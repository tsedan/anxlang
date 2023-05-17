#include "lexer.h"

//===---------------------------------------------------------------------===//
// Lexer - This module tokenizes an input Anx file.
//===---------------------------------------------------------------------===//

std::vector<std::string> lex::src; // source code
lex::Token lex::tok;               // current token being parsed

anx::Pos lex::c, lex::l;
size_t lex::ls;
anx::Pos t;

char grab() {
  static std::string curr = "";

  while (t.c >= curr.size()) {
    std::getline(*anx::stream, curr);
    curr += '\n';

    lex::src.push_back(curr);

    if (anx::stream->eof())
      return EOF;

    t.c = 0;
    t.r++;
  }

  return curr[t.c++];
}

// Get the next token from the input file and update the global token variable
void lex::eat() {
  static char lch = ' ';

  while (isspace(lch))
    lch = grab();

  l.r = c.r, l.c = c.c, ls = tok.val.size();
  c.r = t.r, c.c = t.c - 1;

  tok.val = lch;

  if (lch == EOF) {
    tok.tok = tok_eof;
    return;
  }

  if (isalpha(lch) || lch == '_' || lch == '@') {
    while (isalnum(lch = grab()) || lch == '_')
      tok.val += lch;

    if (tok.val == "fn")
      tok.tok = tok_fn;
    else if (tok.val == "pub")
      tok.tok = tok_pub;
    else if (tok.val == "ret")
      tok.tok = tok_ret;
    else if (tok.val == "var")
      tok.tok = tok_var;
    else if (tok.val == "if")
      tok.tok = tok_if;
    else if (tok.val == "else")
      tok.tok = tok_else;
    else if (tok.val == "while")
      tok.tok = tok_while;
    else if (tok.val == "break")
      tok.tok = tok_break;
    else if (tok.val == "continue")
      tok.tok = tok_cont;
    else
      tok.tok = tok_identifier;

    return;
  }

  if (isdigit(lch)) {
    if (lch == '0') {
      lch = grab();

      if (lch == 'x') {
        do {
          tok.val += lch;
          lch = grab();
        } while (isxdigit(lch) || lch == '_');

        if (lch == '.')
          anx::perr("hexadecimal float literal is not supported", c,
                    tok.val.size());

        if (lch == 'i' || lch == 'u') {
          do
            tok.val += lch;
          while (isdigit(lch = grab()));
        }

        tok.tok = tok_number;
        return;
      } else if (lch == 'b') {
        do {
          tok.val += lch;
          lch = grab();
        } while (lch == '0' || lch == '1' || lch == '_');

        if (isdigit(lch))
          anx::perr("invalid digit in binary literal", {t.r, t.c - 1});

        if (lch == '.' || lch == 'f')
          anx::perr("binary float literal is not supported", c, tok.val.size());

        if (lch == 'i' || lch == 'u') {
          do
            tok.val += lch;
          while (isdigit(lch = grab()));
        }

        tok.tok = tok_number;
        return;
      } else if (lch == 'o') {
        do {
          tok.val += lch;
          lch = grab();
        } while ((lch >= '0' && lch <= '7') || lch == '_');

        if (isdigit(lch))
          anx::perr("invalid digit in octal literal", {t.r, t.c - 1});

        if (lch == '.' || lch == 'f')
          anx::perr("octal float literal is not supported", c, tok.val.size());

        if (lch == 'i' || lch == 'u') {
          do
            tok.val += lch;
          while (isdigit(lch = grab()));
        }

        tok.tok = tok_number;
        return;
      } else if (isdigit(lch) || lch == '.' || lch == '_') {
        tok.val += lch;
      } else
        goto numtype;
    }

    while (isdigit(lch = grab()) || lch == '.' || lch == '_')
      tok.val += lch;

  numtype:
    if (lch == 'i' || lch == 'u' || lch == 'f') {
      do
        tok.val += lch;
      while (isdigit(lch = grab()));
    }

    tok.tok = tok_number;
    return;
  }

  if (lch == '#') {
    do
      lch = grab();
    while (lch != EOF && lch != '\n' && lch != '\r');

    if (lch == EOF)
      tok.tok = tok_eof;
    else
      eat();

    return;
  }

  char old = lch;
  lch = grab();

  switch (old) {
  case ';':
    tok.tok = tok_eol;
    return;
  case ',':
    tok.tok = tok_comma;
    return;
  case '{':
    tok.tok = tok_curlys;
    return;
  case '}':
    tok.tok = tok_curlye;
    return;
  case '(':
    tok.tok = tok_parens;
    return;
  case ')':
    tok.tok = tok_parene;
    return;
  case ':':
    tok.tok = tok_colon;
    return;
  case '*':
  case '/':
  case '+':
  case '-':
  case '%':
    tok.tok = tok_binop;
    return;
  case '<':
  case '>':
    tok.tok = tok_binop;
    if (lch == '=') {
      tok.val += lch;
      lch = grab();
    }
    return;
  case '=':
    if (lch == '=') {
      tok.val += lch;
      lch = grab();
      tok.tok = tok_binop;
    } else
      tok.tok = tok_assign;
    return;
  case '!':
    if (lch == '=') {
      tok.val += lch;
      lch = grab();
      tok.tok = tok_binop;
    } else
      tok.tok = tok_unop;
    return;
  case '\'':
    size_t st = c.c;

    switch (lch) {
    case '\'':
      anx::perr("cannot have empty character literal", {t.r, c.c}, 2);
    case '\\':
      lch = grab();
      switch (lch) {
      case '0':
        tok.val = '\0';
        break;
      case 'n':
        tok.val = '\n';
        break;
      case '\'':
        tok.val = '\'';
        break;
      default:
        anx::perr("unrecognized escape character", {t.r, t.c - 1});
      }
      break;
    default:
      tok.val = lch;
    }

    lch = grab();

    if (lch != '\'')
      anx::perr(
          "missing apostrophe or literal too large for a single character",
          {c.r, st}, t.c - st);
    lch = grab();

    tok.tok = tok_character;
    return;
  }

  anx::perr("invalid token found", c);
}

void lex::exp(lex::TokEnum token, std::string msg) {
  if (tok.tok != token)
    anx::perr(msg, {l.r, l.c + ls});
}
